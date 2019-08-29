#include "pixeltmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267)
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

struct OutsideImageBox
{
    OutsideImageBox(const int w, const int h) : width(w), height(h){}
    bool operator()(const cv::Point &point) 
    {
        if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height)
        { 
            return true;
        } 
        else 
        { 
            return false;
        }
    }
    const int width;
    const int height;
};

struct SADTopLayerScaner
{
    SADTopLayerScaner(const PixelTemplate *const pixTmpl, const SpamRunList *const r, const int s) : tmpl(pixTmpl), roi(r), sad(s) {}
    SADTopLayerScaner(SADTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), roi(s.roi), sad(s.sad) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(SADTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }

    const int sad;
    const PixelTemplate *const tmpl;
    const SpamRunList *const roi;
    PixelTemplate::CandidateList candidates;
};

struct NCCTopLayerScaner
{
    using NCCValueList = std::vector<uint32_t, tbb::scalable_allocator<uint32_t>>;
    NCCTopLayerScaner(const PixelTemplate *const pixTmpl, const double s) : tmpl(pixTmpl), score(s) {}
    NCCTopLayerScaner(NCCTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), score(s.score) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(NCCTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }

    static void sumVector(const NCCValueList &vec, int64_t &sum, int64 &sqrSum);
    static int64_t dotSumVector(const NCCValueList &vec1, const NCCValueList &vec2);

    const double score;
    const PixelTemplate *const tmpl;
    PixelTemplate::CandidateList candidates;
};

template<bool TouchBorder>
struct BFNCCTopLayerScaner
{
    using NCCValueList = std::vector<uint32_t, tbb::scalable_allocator<uint32_t>>;
    BFNCCTopLayerScaner(const PixelTemplate *const pixTmpl, const SpamRun *const r, const double s) : tmpl(pixTmpl), roi(r), score(s) {}
    BFNCCTopLayerScaner(BFNCCTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), roi(s.roi), score(s.score) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(BFNCCTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }

    const double score;
    const PixelTemplate *const tmpl;
    const SpamRun *const roi;
    PixelTemplate::CandidateList candidates;
};

struct SADCandidateScaner
{
    SADCandidateScaner(PixelTemplate *const pixTmpl, const int s, const int l) : tmpl(pixTmpl), sad(s), layer(l) {}
    void operator()(const tbb::blocked_range<int>& r) const;
    const int sad;
    const int layer;
    PixelTemplate *const tmpl;
};

template<bool TouchBorder>
struct BFNCCCandidateScaner
{
    BFNCCCandidateScaner(PixelTemplate *const pixTmpl, const double s, const int l) : tmpl(pixTmpl), score(s), layer(l) {}
    void operator()(const tbb::blocked_range<int>& r) const;
    const double score;
    const int layer;
    PixelTemplate *const tmpl;
};

void SADTopLayerScaner::operator()(const tbb::blocked_range<int>& br)
{
    const cv::Mat &topLayer = tmpl->pyrs_.back();
    const int nCols = topLayer.cols;
    constexpr int32_t simdSize = 16;
    std::array<int16_t, simdSize> tempData;
    const LayerTemplData &topLayerTemplData = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(topLayer.cols, topLayer.rows);
    const int topSad = sad + 5 * (tmpl->pyramid_level_ - 1);
    const int rowStart = br.begin();
    const int rowEnd = br.end();

    for (int row = rowStart; row < rowEnd; ++row)
    {
        for (int col = 0; col < nCols; ++col)
        {
            cv::Point originPt{ col, row };
            int32_t minSAD = std::numeric_limits<int32_t>::max();
            int bestTmplIndex = 0;
            const auto &tmplDatas = boost::get<PixelTemplDatas>(topLayerTemplData.tmplDatas);
            for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
            {
                const PixelTemplData &ptd = tmplDatas[t];

                if (oib(originPt + ptd.minPoint) || oib(originPt + ptd.maxPoint))
                {
                    continue;
                }

                const int32_t numPoints = static_cast<int32_t>(ptd.pixlLocs.size());
                const int32_t regularSize = numPoints & (-simdSize);
                const auto &pixlVals = ptd.pixlVals;
                const cv::Point *pPixlLocs = ptd.pixlLocs.data();

                int32_t n = 0;
                int32_t partialSum = 0;
                for (; n < regularSize; n += simdSize)
                {
                    for (int m = 0; m < simdSize; ++m)
                    {
                        const int x = pPixlLocs->x + col;
                        const int y = pPixlLocs->y + row;
                        tempData[m] = tmpl->row_ptrs_[y][x];
                        pPixlLocs += 1;
                    }

                    vcl::Vec16s tempVec0, tempVec1;
                    tempVec0.load(tempData.data());
                    tempVec1.load(pixlVals.data() + n);
                    partialSum += vcl::horizontal_add(vcl::abs(tempVec0 - tempVec1));
                    if (partialSum > numPoints * topSad)
                    {
                        break;
                    }
                }

                if (n < regularSize)
                {
                    continue;
                }

                for (; n < numPoints; ++n)
                {
                    const int x = pPixlLocs->x + col;
                    const int y = pPixlLocs->y + row;
                    partialSum += std::abs(pixlVals[n] - tmpl->row_ptrs_[y][x]);
                    pPixlLocs += 1;
                }

                int32_t sad = partialSum / numPoints;
                if (sad < minSAD)
                {
                    minSAD = sad;
                    bestTmplIndex = t;
                }
            }

            if (minSAD < topSad)
            {
                candidates.emplace_back(row, col, bestTmplIndex, static_cast<float>(255 - minSAD));
            }
        }
    }
}

inline void NCCTopLayerScaner::sumVector(const NCCValueList &vec, int64_t &sum, int64 &sqrSum)
{
    vcl::Vec8ui sumVec(0);
    vcl::Vec8ui sqrSumVec(0);
    const uint32_t *pVals = vec.data();
    constexpr int simdSize = 8;
    const int numItems = static_cast<int>(vec.size());
    for (int n = 0; n < numItems; n += simdSize)
    {
        vcl::Vec8ui tempVec;
        tempVec.load(pVals);
        sumVec += tempVec;
        sqrSumVec += (tempVec * tempVec);
        pVals += 8;
    }

    sum = vcl::horizontal_add(sumVec);
    sqrSum = vcl::horizontal_add(sqrSumVec);
}

inline int64_t NCCTopLayerScaner::dotSumVector(const NCCValueList &vec1, const NCCValueList &vec2)
{
    vcl::Vec8ui sumVec(0);
    const uint32_t *pVals1 = vec1.data();
    const uint32_t *pVals2 = vec2.data();
    constexpr int simdSize = 8;
    const int numItems = static_cast<int>(vec1.size());
    for (int n = 0; n < numItems; n += simdSize)
    {
        vcl::Vec8ui tempVec1, tempVec2;
        tempVec1.load(pVals1);
        tempVec2.load(pVals2);
        sumVec += (tempVec1 * tempVec2);
        pVals1 += 8;
        pVals2 += 8;
    }

    return vcl::horizontal_add(sumVec);
}

void NCCTopLayerScaner::operator()(const tbb::blocked_range<int>& br)
{
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerTemplData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const double layerMinScore = std::max(0.5, score - 0.05 * (tmpl->pyramid_level_ - 1));
    const int rowStart = br.begin();
    const int rowEnd = br.end();
    const NCCTemplDatas &tmplDatas = boost::get<NCCTemplDatas>(layerTempls.tmplDatas);
    const int numLayerTempls = static_cast<int>(tmplDatas.size());

    constexpr int simdSize = 8;
    std::vector<uint32_t, tbb::scalable_allocator<uint32_t>> tmplAVals, tmplBVals;
    std::vector<uint32_t, tbb::scalable_allocator<uint32_t>> partAVals[kTP_WaveGuard], partBVals[kTP_WaveGuard];

    std::map<int, PixelTemplate::Candidate, std::less<int>, tbb::scalable_allocator<std::pair<const int, PixelTemplate::Candidate>>> aCandidates;
    for (int t=0; t<numLayerTempls; ++t)
    {
        const NCCTemplData &ntd  = tmplDatas[t];
        const int64_t numPartA   = static_cast<int64_t>(ntd.partAVals.size());
        const int64_t numPartB   = static_cast<int64_t>(ntd.partBVals.size());
        const int64_t numPoints  = numPartA + numPartB;
        const int64_t tmplSum    = ntd.partASum + ntd.partBSum;
        const int64_t tmplSqrSum = ntd.partASqrSum + ntd.partBSqrSum;
        tmplAVals.resize((static_cast<int>(ntd.partAVals.size()) + simdSize - 1) & (-simdSize), 0);
        tmplBVals.resize((static_cast<int>(ntd.partBVals.size()) + simdSize - 1) & (-simdSize), 0);

        uint32_t *pTmplAVals = tmplAVals.data();
        uint32_t *pTmplBVals = tmplBVals.data();
        for (uint8_t v : ntd.partAVals)
        {
            *pTmplAVals = v; pTmplAVals += 1;
        }

        for (uint8_t v : ntd.partBVals)
        {
            *pTmplBVals = v; pTmplBVals += 1;
        }

        partAVals[kTP_WaveBack].resize((ntd.cPartABoundaries + simdSize - 1) & (-simdSize), 0);
        partAVals[kTP_WaveFront].resize((ntd.cPartABoundaries + simdSize - 1) & (-simdSize), 0);
        partAVals[kTP_WaveMiddle].resize((static_cast<int>(ntd.partAVals.size()) + simdSize - 1) & (-simdSize), 0);
        partBVals[kTP_WaveBack].resize((static_cast<int>(ntd.partBBoundaries.size()) + simdSize - 1) & (-simdSize), 0);
        partBVals[kTP_WaveFront].resize((static_cast<int>(ntd.partBBoundaries.size()) + simdSize - 1) & (-simdSize), 0);
        partBVals[kTP_WaveMiddle].resize((static_cast<int>(ntd.partBVals.size()) + simdSize - 1) & (-simdSize), 0);

        for (int row = rowStart; row < rowEnd; ++row)
        {
            int64_t partASum = -1, partABackSum = -1, partAFrontSum = -1;
            int64_t partASqrSum = -1, partABackSqrSum = -1, partAFrontSqrSum = -1;
            int64_t partBSum = -1, partBBackSum = -1, partBFrontSum = -1;
            int64_t partBSqrSum = -1, partBBackSqrSum = -1, partBFrontSqrSum = -1;

            for (int col = 0; col < nLayerCols; ++col)
            {
                cv::Point originPt{ col, row };
                if (oib(originPt + ntd.minPoint) || oib(originPt + ntd.maxPoint))
                {
                    continue;
                }

                uint32_t *pPartAVals[kTP_WaveGuard]{ partAVals[kTP_WaveMiddle].data(), partAVals[kTP_WaveBack].data(), partAVals[kTP_WaveFront].data() };
                for (const cv::Point3i &pt3 : ntd.partALocs)
                {
                    const uint8_t val = tmpl->row_ptrs_[pt3.y + row][pt3.x + col];
                    *pPartAVals[kTP_WaveMiddle] = val; pPartAVals[kTP_WaveMiddle] += 1;
                    if (pt3.z) { *pPartAVals[pt3.z] = val; pPartAVals[pt3.z] += 1; }
                }

                if (partASum < 0)
                {
                    uint32_t *pPartBVals[kTP_WaveGuard]{ partBVals[kTP_WaveMiddle].data(), partBVals[kTP_WaveBack].data(), partBVals[kTP_WaveFront].data() };
                    for (const cv::Point3i &pt3 : ntd.partBLocs)
                    {
                        const uint8_t val = tmpl->row_ptrs_[pt3.y + row][pt3.x + col];
                        *pPartBVals[kTP_WaveMiddle] = val; pPartBVals[kTP_WaveMiddle] += 1;
                        if (pt3.z) { *pPartBVals[pt3.z] = val; pPartBVals[pt3.z] += 1; }
                    }

                    sumVector(partAVals[kTP_WaveMiddle], partASum, partASqrSum);
                    sumVector(partAVals[kTP_WaveBack], partABackSum, partABackSqrSum);
                    sumVector(partAVals[kTP_WaveFront], partAFrontSum, partAFrontSqrSum);
                    sumVector(partBVals[kTP_WaveMiddle], partBSum, partBSqrSum);
                    sumVector(partBVals[kTP_WaveBack], partBBackSum, partBBackSqrSum);
                    sumVector(partBVals[kTP_WaveFront], partBFrontSum, partBFrontSqrSum);
                }
                else
                {
                    uint32_t *partBBackVals = partBVals[kTP_WaveBack].data();
                    uint32_t *partBFrontVals = partBVals[kTP_WaveFront].data();
                    for (const std::pair<cv::Point, cv::Point> &pts : ntd.partBBoundaries)
                    {
                        *partBBackVals = tmpl->row_ptrs_[pts.first.y + row][pts.first.x + col];
                        partBBackVals += 1;

                        *partBFrontVals = tmpl->row_ptrs_[pts.second.y + row][pts.second.x + col];
                        partBFrontVals += 1;
                    }

                    sumVector(partAVals[kTP_WaveFront], partAFrontSum, partAFrontSqrSum);
                    sumVector(partBVals[kTP_WaveFront], partBFrontSum, partBFrontSqrSum);

                    partASum = partASum - partABackSum + partAFrontSum;
                    partASqrSum = partASqrSum - partABackSqrSum + partAFrontSqrSum;
                    partBSum = partBSum - partBBackSum + partBFrontSum;
                    partBSqrSum = partBSqrSum - partBBackSqrSum + partBFrontSqrSum;

                    sumVector(partAVals[kTP_WaveBack], partABackSum, partABackSqrSum);
                    sumVector(partBVals[kTP_WaveBack], partBBackSum, partBBackSqrSum);
                }

                const int64_t partSum    = partASum + partBSum;
                const int64_t partSqrSum = partASqrSum + partBSqrSum;

                int64_t partADot = dotSumVector(tmplAVals, partAVals[kTP_WaveMiddle]);
                double val1 = PixelTemplate::calcValue1(partSqrSum, partSum, numPoints);
                if (val1 < 0)
                {
                    continue;
                }

                double val2 = PixelTemplate::calcValue2(partBSqrSum, partSum, partBSum, numPoints, numPartB);
                if (val2 < 0)
                {
                    continue;
                }

                int64_t partA_A = numPoints*tmplSum*partASum + numPoints*partSum*ntd.partASum - numPartA*tmplSum*partSum;
                double partAVal = partADot - partA_A / static_cast<double>(numPoints*numPoints);
                double nccUpper = (partAVal + ntd.betaz*val2) / (ntd.norm*val1);

                if (nccUpper < layerMinScore)
                {
                    continue;
                }

                uint32_t *partBMiddleVals = partBVals[kTP_WaveMiddle].data();
                for (const cv::Point3i &pt3 : ntd.partBLocs)
                {
                    *partBMiddleVals = tmpl->row_ptrs_[pt3.y + row][pt3.x + col];
                    partBMiddleVals += 1;
                }

                int64_t partDot = partADot + dotSumVector(tmplBVals, partBVals[kTP_WaveMiddle]);
                double ncc = (partDot - static_cast<double>(tmplSum * partSum) / numPoints) / (ntd.norm*val1);
                if (ncc >= layerMinScore)
                {
                    PixelTemplate::Candidate candidate{row, col, t, static_cast<float>(ncc)};
                    auto insIt = aCandidates.emplace(row*nLayerCols+col, candidate);
                    if (!insIt.second)
                    {
                        if (candidate.score > insIt.first->second.score)
                        {
                            insIt.first->second.score = candidate.score;
                            insIt.first->second.mindex = t;
                        }
                    }
                }
            }
        }
    }

    candidates.reserve(aCandidates.size());
    for (const auto &cItem : aCandidates)
    {
        candidates.push_back(cItem.second);
    }
}

template<bool TouchBorder>
void BFNCCTopLayerScaner<TouchBorder>::operator()(const tbb::blocked_range<int>& br)
{
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerTemplData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const double layerMinScore = std::max(0.5, score - 0.1 * (tmpl->pyramid_level_ - 1));
    const int runStart = br.begin();
    const int runEnd = br.end();
    const BFNCCTemplDatas &tmplDatas = boost::get<BFNCCTemplDatas>(layerTempls.tmplDatas);
    const int numLayerTempls = static_cast<int>(tmplDatas.size());

    std::vector<uint32_t, tbb::scalable_allocator<uint32_t>> partVals;
    std::map<int, PixelTemplate::Candidate, std::less<int>, tbb::scalable_allocator<std::pair<const int, PixelTemplate::Candidate>>> aCandidates;

    for (int t = 0; t < numLayerTempls; ++t)
    {
        const BruteForceNCCTemplData &ntd = tmplDatas[t];
        const int64_t numPoints  = static_cast<int64_t>(ntd.vals.size());
        const int64_t tmplSum    = ntd.sum;
        partVals.resize(0);
        partVals.resize(ntd.regVals.size(), 0);

        for (int run = runStart; run < runEnd; ++run)
        {
            const int row = roi[run].row;
            const int colStart = roi[run].colb;
            const int colEnd = roi[run].cole;

            for (int col = colStart; col < colEnd; ++col)
            {
                cv::Point originPt{ col, row };
                uint32_t *pPartVals = partVals.data();

                if (TouchBorder) {
                    if (oib(originPt + ntd.minPoint) || oib(originPt + ntd.maxPoint))
                    {
                        for (const cv::Point &pt : ntd.locs)
                        {
                            cv::Point tPt{ pt.x + col, pt.y + row };
                            if (!oib(tPt)) {
                                *pPartVals = tmpl->row_ptrs_[tPt.y][tPt.x];
                            }
                            pPartVals += 1;
                        }
                    }
                    else {
                        for (const cv::Point &pt : ntd.locs)
                        {
                            *pPartVals = tmpl->row_ptrs_[pt.y + row][pt.x + col];
                            pPartVals += 1;
                        }
                    }
                } else {
                    if (oib(originPt + ntd.minPoint) || oib(originPt + ntd.maxPoint)) {
                        continue;
                    }
                    for (const cv::Point &pt : ntd.locs)
                    {
                        *pPartVals = tmpl->row_ptrs_[pt.y + row][pt.x + col];
                        pPartVals += 1;
                    }
                }

                int64_t partSum = 0, partSqrSum = 0;
                NCCTopLayerScaner::sumVector(partVals, partSum, partSqrSum);

                double val1 = PixelTemplate::calcValue1(partSqrSum, partSum, numPoints);
                if (val1 < 0)
                {
                    continue;
                }

                int64_t dotProduct = NCCTopLayerScaner::dotSumVector(ntd.regVals, partVals);
                double ncc = (dotProduct - static_cast<double>(tmplSum * partSum) / numPoints) / (ntd.norm*val1);
                if (ncc >= layerMinScore)
                {
                    PixelTemplate::Candidate candidate{ row, col, t, static_cast<float>(ncc) };
                    auto insIt = aCandidates.emplace(row*nLayerCols + col, candidate);
                    if (!insIt.second)
                    {
                        if (candidate.score > insIt.first->second.score)
                        {
                            insIt.first->second.score = candidate.score;
                            insIt.first->second.mindex = t;
                        }
                    }
                }
            }
        }
    }

    candidates.reserve(aCandidates.size());
    for (const auto &cItem : aCandidates)
    {
        candidates.push_back(cItem.second);
    }
}

void SADCandidateScaner::operator()(const tbb::blocked_range<int>& r) const
{
    LayerTemplData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    constexpr int32_t simdSize = 16;
    std::array<int16_t, simdSize> tempData;
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const int layerSad = sad + 5 * layer;
    const auto &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
    const auto &upperTmplDatas = boost::get<PixelTemplDatas>(tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas);

    for (int c=r.begin(); c!=r.end(); ++c)
    {
        PixelTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        cv::Point originPt{ col, row };
        int32_t minSAD = std::numeric_limits<int32_t>::max();
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;
        for (const int tmplIndex : tmplIndices)
        {
            const PixelTemplData &ptd = tmplDatas[tmplIndex];

            if (oib(originPt + ptd.minPoint) || oib(originPt + ptd.maxPoint))
            {
                continue;
            }

            const int32_t numPoints = static_cast<int32_t>(ptd.pixlLocs.size());
            const int32_t regularSize = numPoints & (-simdSize);
            const auto &pixlVals = ptd.pixlVals;
            const cv::Point *pPixlLocs = ptd.pixlLocs.data();

            int32_t n = 0;
            int32_t partialSum = 0;
            for (; n < regularSize; n += simdSize)
            {
                for (int m = 0; m < simdSize; ++m)
                {
                    const int x = pPixlLocs->x + col;
                    const int y = pPixlLocs->y + row;
                    tempData[m] = tmpl->row_ptrs_[y][x];
                    pPixlLocs += 1;
                }

                vcl::Vec16s tempVec0, tempVec1;
                tempVec0.load(tempData.data());
                tempVec1.load(pixlVals.data() + n);
                partialSum += vcl::horizontal_add(vcl::abs(tempVec0 - tempVec1));
                if (partialSum > numPoints * layerSad)
                {
                    break;
                }
            }

            if (n < regularSize)
            {
                continue;
            }

            for (; n < numPoints; ++n)
            {
                const int x = pPixlLocs->x + col;
                const int y = pPixlLocs->y + row;
                partialSum += std::abs(pixlVals[n] - tmpl->row_ptrs_[y][x]);
                pPixlLocs += 1;
            }

            int32_t sad = partialSum / numPoints;
            if (sad < minSAD)
            {
                minSAD = sad;
                bestTmplIndex = tmplIndex;
            }
        }

        if (minSAD < layerSad)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = static_cast<float>(255 - minSAD);
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
        }
    }
}

template<bool TouchBorder>
void BFNCCCandidateScaner<TouchBorder>::operator()(const tbb::blocked_range<int>& r) const
{
    LayerTemplData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const double layerScore = std::max(0.5, score - 0.1 * layer);
    const auto &tmplDatas = boost::get<BFNCCTemplDatas>(ltd.tmplDatas);
    const auto &upperTmplDatas = boost::get<BFNCCTemplDatas>(tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas);

    std::vector<uint32_t, tbb::scalable_allocator<uint32_t>> partVals;
    for (int c = r.begin(); c != r.end(); ++c)
    {
        PixelTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        cv::Point originPt{ col, row };
        double maxScore = -1;
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;
        for (const int tmplIndex : tmplIndices)
        {
            const BruteForceNCCTemplData &ntd = tmplDatas[tmplIndex];
            partVals.resize(0);
            partVals.resize(ntd.regVals.size(), 0);
            uint32_t *pPartVals = partVals.data();

            if (TouchBorder) {
                if (oib(originPt + ntd.minPoint) || oib(originPt + ntd.maxPoint))
                {
                    for (const cv::Point &pt : ntd.locs)
                    {
                        cv::Point tPt{ pt.x + col, pt.y + row };
                        if (!oib(tPt)) {
                            *pPartVals = tmpl->row_ptrs_[tPt.y][tPt.x];
                        }
                        pPartVals += 1;
                    }
                } else {
                    for (const cv::Point &pt : ntd.locs)
                    {
                        *pPartVals = tmpl->row_ptrs_[pt.y + row][pt.x + col];
                        pPartVals += 1;
                    }
                }
            } else {
                if (oib(originPt + ntd.minPoint) || oib(originPt + ntd.maxPoint)) {
                    continue;
                }
                for (const cv::Point &pt : ntd.locs)
                {
                    *pPartVals = tmpl->row_ptrs_[pt.y + row][pt.x + col];
                    pPartVals += 1;
                }
            }

            int64_t partSum = 0, partSqrSum = 0;
            NCCTopLayerScaner::sumVector(partVals, partSum, partSqrSum);

            const int64_t numPoints = static_cast<int64_t>(ntd.vals.size());
            double val1 = PixelTemplate::calcValue1(partSqrSum, partSum, numPoints);
            if (val1 < 0)
            {
                continue;
            }

            int64_t dotProduct = NCCTopLayerScaner::dotSumVector(ntd.regVals, partVals);
            double ncc = (dotProduct - static_cast<double>(ntd.sum * partSum) / numPoints) / (ntd.norm*val1);
            if (ncc > maxScore)
            {
                maxScore = ncc;
                bestTmplIndex = tmplIndex;
            }
        }

        if (maxScore > layerScore)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = static_cast<float>(maxScore);
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
        }
    }
}

PixelTemplate::CandidateGroup::CandidateGroup(CandidateList &candidates)
{
    RLEncodeCandidates(candidates);
}

void PixelTemplate::CandidateGroup::RLEncodeCandidates(CandidateList &candidates)
{
    resize(0);
    std::sort(candidates.begin(), candidates.end(), [](const Candidate &l, const Candidate &r)
    {
        return (l.row < r.row) || (l.row == r.row && l.col < r.col);
    });

    if (!candidates.empty())
    {
        constexpr int negInf = std::numeric_limits<int>::min();
        int colb = candidates.front().col;
        int preRow = candidates.front().row;
        int preCol = candidates.front().col - 1;
        Candidate bestCandidate = candidates.front();
        for (const Candidate &candidate : candidates)
        {
            if (candidate.row != preRow)
            {
                emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
                colb = candidate.col;
                bestCandidate = candidate;
            }
            else
            {
                if (candidate.col != (preCol + 1))
                {
                    emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
                    colb = candidate.col;
                    bestCandidate = candidate;
                }
            }

            preRow = candidate.row;
            preCol = candidate.col;

            if (candidate.score > bestCandidate.score)
            {
                bestCandidate = candidate;
            }
        }

        emplace_back(preRow, colb, preCol + 1).best = bestCandidate;
    }
}

void PixelTemplate::CandidateGroup::Connect(CandidateGroupList &candidateGroups)
{
    row_ranges.resize(0);
    adjacency_list.resize(0);
    adjacency_list.resize(size());
    candidateGroups.resize(0);

    if (!empty())
    {
        int begIdx = 0;
        int16_t currentRow = front().row;
        row_ranges.reserve(size());

        const CandidateRun *runs = data();
        const int numRuns = static_cast<int>(size());
        for (int run = 0; run < numRuns; ++run)
        {
            if (runs[run].row != currentRow)
            {
                row_ranges.emplace_back(currentRow, begIdx, run);
                begIdx = run;
                currentRow = runs[run].row;
            }
        }
        row_ranges.emplace_back(currentRow, begIdx, numRuns);

        const int endRow = static_cast<int>(row_ranges.size() - 1);
        for (auto row = 0; row < endRow; ++row)
        {
            const RowRange &rowRange = row_ranges[row];
            const RowRange &nextRowRange = row_ranges[row + 1];

            if ((rowRange.row + 1) == nextRowRange.row)
            {
                int nextBegIdx = nextRowRange.beg;
                const int nextEndIdx = nextRowRange.end;
                for (int runIdx = rowRange.beg; runIdx != rowRange.end; ++runIdx)
                {
                    bool metInter = false;
                    for (int nextRunIdx = nextBegIdx; nextRunIdx != nextEndIdx; ++nextRunIdx)
                    {
                        if (runs[runIdx].IsColumnIntersection(runs[nextRunIdx]))
                        {
                            metInter = true;
                            nextBegIdx = nextRunIdx;
                            adjacency_list[runIdx].push_back(nextRunIdx);
                            adjacency_list[nextRunIdx].push_back(runIdx);
                        }
                        else if (metInter)
                        {
                            break;
                        }
                    }
                }
            }
        }

        run_stack.resize(0);
        rgn_idxs.resize(0);
        std::vector<uint8_t> met(adjacency_list.size());
        for (int n = 0; n < numRuns; ++n)
        {
            if (!met[n])
            {
                rgn_idxs.emplace_back();
                run_stack.push_back(n);
                while (!run_stack.empty())
                {
                    const int seedRun = run_stack.back();
                    run_stack.pop_back();
                    if (!met[seedRun])
                    {
                        met[seedRun] = 1;
                        rgn_idxs.back().push_back(seedRun);
                        for (const int a : adjacency_list[seedRun])
                        {
                            run_stack.push_back(a);
                        }
                    }
                }

                std::sort(rgn_idxs.back().begin(), rgn_idxs.back().end());
            }
        }

        candidateGroups.resize(rgn_idxs.size());
        const int numRgns = static_cast<int>(rgn_idxs.size());
        for (int r = 0; r < numRgns; ++r)
        {
            CandidateGroup &rgn = candidateGroups[r];
            const std::vector<int> &rgnIdx = rgn_idxs[r];
            const int numRgnRuns = static_cast<int>(rgnIdx.size());
            rgn.resize(numRgnRuns);
            for (int rr = 0; rr < numRgnRuns; ++rr)
            {
                rgn[rr] = runs[rgnIdx[rr]];
            }
        }
    }
}

PixelTemplate::PixelTemplate()
    : pyramid_level_(4)
{
}

PixelTemplate::~PixelTemplate()
{ 
}

SpamResult PixelTemplate::matchPixelTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle)
{
    clearCacheMatchData();

    pos.x = 0.f;
    pos.y = 0.f;
    angle = 0.f;

    if (pyramid_tmpl_datas_.empty() ||
        pyramid_level_ != static_cast<int>(pyramid_tmpl_datas_.size()))
    {
        return SpamResult::kSR_TM_CORRUPTED_TEMPL_DATA;
    }

    cv::buildPyramid(img, pyrs_, pyramid_level_ - 1, cv::BORDER_REFLECT);
    const cv::Mat &topLayer = pyrs_.back();

    const int nTopRows = topLayer.rows;
    const int nTopCols = topLayer.cols;

    row_ptrs_.clear();
    row_ptrs_.resize(nTopRows);
    for (int row = 0; row < nTopRows; ++row)
    {
        row_ptrs_[row] = topLayer.ptr<uint8_t>(row);
    }

    if (top_layer_search_roi_.GetData().empty())
    {
        top_layer_full_domain_.SetRegion(cv::Rect(0, 0, nTopCols, nTopRows));
        SADTopLayerScaner sadScaner(this, &top_layer_full_domain_.GetData(), sad);
        tbb::parallel_reduce(tbb::blocked_range<int>(0, nTopRows), sadScaner);
        candidates_.swap(sadScaner.candidates);
    }
    else
    {
        SADTopLayerScaner sadScaner(this, &top_layer_search_roi_.GetData(), sad);
        tbb::parallel_reduce(tbb::blocked_range<int>(0, nTopRows), sadScaner);
        candidates_.swap(sadScaner.candidates);
    }

    supressNoneMaximum();

    final_candidates_.resize(0);
    const int numTopLayerCandidates = static_cast<int>(candidates_.size());
    for (int cc=0; cc<numTopLayerCandidates; ++cc)
    {
        const Candidate &candidate = candidates_[cc];
        for (int row = -2; row < 3; ++row)
        {
            for (int col = -2; col < 3; ++col)
            {
                final_candidates_.emplace_back(candidate.row*2+row, candidate.col * 2 + col, candidate.mindex, cc);
            }
        }
    }

    const int layerIndex = static_cast<int>(pyramid_tmpl_datas_.size() - 2);
    for (int layer = layerIndex; layer >= 0; --layer)
    {
        const cv::Mat &layerMat = pyrs_[layer];
        const int nLayerRows = layerMat.rows;

        row_ptrs_.resize(0);
        row_ptrs_.resize(nLayerRows);
        for (int row = 0; row < nLayerRows; ++row)
        {
            row_ptrs_[row] = layerMat.ptr<uint8_t>(row);
        }

        tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), SADCandidateScaner(this, sad, layer));

        for (Candidate &candidate : candidates_)
        {
            candidate.score = 0.f;
        }

        for (const Candidate &candidate : final_candidates_)
        {
            if (candidate.mindex >= 0)
            {
                if (candidate.score > candidates_[candidate.label].score)
                {
                    candidates_[candidate.label] = candidate;
                }
            }
        }

        final_candidates_.resize(0);
        for (const Candidate &candidate : candidates_)
        {
            if (candidate.score > 0.f)
            {
                final_candidates_.push_back(candidate);
            }
        }

        if (layer > 0)
        {
            candidates_.swap(final_candidates_);

            final_candidates_.resize(0);
            const int numLayerCandidates = static_cast<int>(candidates_.size());
            for (int cc = 0; cc < numLayerCandidates; ++cc)
            {
                const Candidate &candidate = candidates_[cc];
                for (int row = -2; row < 3; ++row)
                {
                    for (int col = -2; col < 3; ++col)
                    {
                        final_candidates_.emplace_back(candidate.row * 2 + row, candidate.col * 2 + col, candidate.mindex, cc);
                    }
                }
            }
        }
    }

    Candidate bestCandidate(0, 0, std::numeric_limits<float>::lowest());
    for (const Candidate &candidate : final_candidates_)
    {
        if (candidate.score > bestCandidate.score)
        {
            bestCandidate = candidate;
        }
    }

    if (bestCandidate.score > static_cast<float>(255-sad))
    {
        pos.x = static_cast<float>(bestCandidate.col);
        pos.y = static_cast<float>(bestCandidate.row);
        const auto &tmplDatas = boost::get<PixelTemplDatas>(pyramid_tmpl_datas_[0].tmplDatas);
        angle = tmplDatas[bestCandidate.mindex].angle;

        return SpamResult::kSR_SUCCESS;
    }
    else
    {
        return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
    }
}

SpamResult PixelTemplate::matchNCCTemplate(const cv::Mat &img, const float minScore, cv::Point2f &pos, float &angle, float &score)
{
    clearCacheMatchData();

    pos.x = 0.f;
    pos.y = 0.f;
    angle = 0.f;
    score = 0.f;

    if (pyramid_tmpl_datas_.empty() ||
        pyramid_level_ != static_cast<int>(pyramid_tmpl_datas_.size()))
    {
        return SpamResult::kSR_TM_CORRUPTED_TEMPL_DATA;
    }

    cv::buildPyramid(img, pyrs_, pyramid_level_ - 1, cv::BORDER_REFLECT);
    const cv::Mat &topLayer = pyrs_.back();

    const int nTopRows = topLayer.rows;
    const int nTopCols = topLayer.cols;

    row_ptrs_.clear();
    row_ptrs_.resize(nTopRows);
    for (int row = 0; row < nTopRows; ++row)
    {
        row_ptrs_[row] = topLayer.ptr<uint8_t>(row);
    }

    if (cv::TM_CCOEFF == match_mode_ ||
        cv::TM_CCOEFF_NORMED == match_mode_)
    {
        if (top_layer_search_roi_.GetData().empty())
        {
            top_layer_full_domain_.SetRegion(cv::Rect(0, 0, nTopCols, nTopRows));
            const int numRuns = static_cast<int>(top_layer_full_domain_.GetData().size());
            BFNCCTopLayerScaner<true> bfNCCScaner(this, top_layer_full_domain_.GetData().data(), minScore);
            tbb::parallel_reduce(tbb::blocked_range<int>(0, numRuns), bfNCCScaner);
            candidates_.swap(bfNCCScaner.candidates);
        }
        else
        {
            const int numRuns = static_cast<int>(top_layer_search_roi_.GetData().size());
            BFNCCTopLayerScaner<true> bfNCCScaner(this, top_layer_search_roi_.GetData().data(), minScore);
            tbb::parallel_reduce(tbb::blocked_range<int>(0, numRuns), bfNCCScaner);
            candidates_.swap(bfNCCScaner.candidates);
        }
    }

    supressNoneMaximum();

    final_candidates_.resize(0);
    const int numTopLayerCandidates = static_cast<int>(candidates_.size());
    for (int cc = 0; cc < numTopLayerCandidates; ++cc)
    {
        const Candidate &candidate = candidates_[cc];
        for (int row = -2; row < 3; ++row)
        {
            for (int col = -2; col < 3; ++col)
            {
                final_candidates_.emplace_back(candidate.row * 2 + row, candidate.col * 2 + col, candidate.mindex, cc);
            }
        }
    }

    const int layerIndex = static_cast<int>(pyramid_tmpl_datas_.size() - 2);
    for (int layer = layerIndex; layer >= 0; --layer)
    {
        const cv::Mat &layerMat = pyrs_[layer];
        const int nLayerRows = layerMat.rows;

        row_ptrs_.resize(0);
        row_ptrs_.resize(nLayerRows);
        for (int row = 0; row < nLayerRows; ++row)
        {
            row_ptrs_[row] = layerMat.ptr<uint8_t>(row);
        }

        tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), BFNCCCandidateScaner<true>(this, minScore, layer));

        for (Candidate &candidate : candidates_)
        {
            candidate.score = 0.f;
        }

        for (const Candidate &candidate : final_candidates_)
        {
            if (candidate.mindex >= 0)
            {
                if (candidate.score > candidates_[candidate.label].score)
                {
                    candidates_[candidate.label] = candidate;
                }
            }
        }

        final_candidates_.resize(0);
        for (const Candidate &candidate : candidates_)
        {
            if (candidate.score > 0.f)
            {
                final_candidates_.push_back(candidate);
            }
        }

        if (layer > 0)
        {
            candidates_.swap(final_candidates_);

            final_candidates_.resize(0);
            const int numLayerCandidates = static_cast<int>(candidates_.size());
            for (int cc = 0; cc < numLayerCandidates; ++cc)
            {
                const Candidate &candidate = candidates_[cc];
                for (int row = -2; row < 3; ++row)
                {
                    for (int col = -2; col < 3; ++col)
                    {
                        final_candidates_.emplace_back(candidate.row * 2 + row, candidate.col * 2 + col, candidate.mindex, cc);
                    }
                }
            }
        }
    }

    Candidate bestCandidate(0, 0, std::numeric_limits<float>::lowest());
    for (const Candidate &candidate : final_candidates_)
    {
        if (candidate.score > bestCandidate.score)
        {
            bestCandidate = candidate;
        }
    }

    if (bestCandidate.score > minScore)
    {
        pos.x = static_cast<float>(bestCandidate.col);
        pos.y = static_cast<float>(bestCandidate.row);
        const auto &tmplDatas = boost::get<BFNCCTemplDatas>(pyramid_tmpl_datas_[0].tmplDatas);
        angle = tmplDatas[bestCandidate.mindex].angle;
        score = bestCandidate.score;

        return SpamResult::kSR_SUCCESS;
    }
    else
    {
        return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
    }
}

SpamResult PixelTemplate::CreateTemplate(const PixelTmplCreateData &createData)
{
    destroyData();
    SpamResult sr = verifyCreateData(createData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    sr = fastCreateTemplate(createData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    linkTemplatesBetweenLayers();

    pyramid_level_ = createData.pyramidLevel;
    match_mode_    = createData.matchMode;

    if (cv::TM_CCOEFF == match_mode_ ||
        cv::TM_CCOEFF_NORMED == match_mode_)
    {
        sr = changeToBruteForceNCCTemplate();
        if (SpamResult::kSR_SUCCESS != sr)
        {
            destroyData();
            return sr;
        }
    }

    processToplayerSearchROI(createData);

    return SpamResult::kSR_SUCCESS;
}

void PixelTemplate::destroyData()
{
    pyramid_level_ = 1;
    match_mode_ = cv::TM_SQDIFF;
    cfs_.clear();
    tmpl_rgns_.clear();
    top_layer_search_roi_.clear();
    top_layer_search_roi_g_.clear();
    pyramid_tmpl_datas_.clear();
    pyrs_.clear();
}

void PixelTemplate::clearCacheMatchData()
{
    row_ptrs_.resize(0);
    candidates_.resize(0);
    top_candidates_.resize(0);
    final_candidates_.resize(0);
    candidate_runs_.resize(0);
    candidate_groups_.resize(0);
}

SpamResult PixelTemplate::verifyCreateData(const PixelTmplCreateData &createData)
{
    if (createData.pyramidLevel < 1)
    {
        return SpamResult::kSR_TM_PYRAMID_LEVEL_INVALID;
    }

    if (createData.srcImg.empty())
    {
        return SpamResult::kSR_IMG_EMPTY;
    }

    int topWidth  = createData.srcImg.cols;
    int topHeight = createData.srcImg.rows;
    for (int n=0; n<(createData.pyramidLevel-1); ++n)
    {
        topWidth /= 2;
        topHeight /= 2;
    }

    if (0==topWidth || 0==topHeight)
    {
        return SpamResult::kSR_TM_PYRAMID_LEVEL_TOO_LARGE;
    }

    if (createData.angleExtent>360 || createData.angleExtent<0)
    {
        return SpamResult::kSR_TM_ANGLE_RANGE_INVALID;
    }

    return SpamResult::kSR_OK;
}

SpamResult PixelTemplate::calcCreateTemplate(const PixelTmplCreateData &createData)
{
    if (createData.tmplRgn.empty())
    {
        return SpamResult::kSR_TM_EMPTY_TEMPL_REGION;
    }

    std::vector<Geom::PathVector> tmplRgns;
    tmplRgns.push_back(createData.tmplRgn);

    double s = 0.5;
    for (int l=1; l < createData.pyramidLevel; ++l)
    {
        tmplRgns.push_back(createData.tmplRgn*Geom::Scale(s, s));
        s *= 0.5;
    }

    cv::Mat tmplImg;
    cv::buildPyramid(createData.srcImg, pyrs_, createData.pyramidLevel-1, cv::BORDER_REFLECT);
    SpamRgn maskRgn(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(0, 0), 5))));
    PointSet maskPoints(maskRgn);

    cv::Mat transPyrImg;
    cv::Mat transRotPyrImg;
    cv::Mat transMat = (cv::Mat_<double>(2, 3) << 1, 0, 0, 0, 1, 0);
    std::vector<uint8_t> maskBuf;

    for (int l=0; l < createData.pyramidLevel; ++l)
    {
        SpamRgn rgn(tmplRgns[l], maskBuf);
        cv::Point anchorPoint{cvRound(rgn.Centroid().x), cvRound(rgn.Centroid().y)};
        cfs_.emplace_back(static_cast<float>(anchorPoint.x), static_cast<float>(anchorPoint.y));

        Geom::Circle minCircle = rgn.MinCircle();
        if (minCircle.radius()<5)
        {
            return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
        }

        const auto angleStep = Geom::deg_from_rad(std::acos(1 - 2 / (minCircle.radius()*minCircle.radius())));
        if (angleStep<0.3)
        {
            return SpamResult::kSR_TM_TEMPL_REGION_TOO_LARGE;
        }

        pyramid_tmpl_datas_.emplace_back(static_cast<float>(angleStep), 0.f);
        LayerTemplData &ltd = pyramid_tmpl_datas_.back();

        const cv::Mat &pyrImg = pyrs_[l];
        const cv::Point pyrImgCenter{ pyrImg.cols / 2, pyrImg.rows / 2 };
        ltd.tmplDatas = PixelTemplDatas();
        auto &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);

        transMat.at<double>(0, 2) = pyrImgCenter.x - anchorPoint.x;
        transMat.at<double>(1, 2) = pyrImgCenter.y - anchorPoint.y;
        cv::warpAffine(pyrImg, transPyrImg, transMat, cv::Size(pyrImg.cols, pyrImg.rows));
        std::map<int, uint8_t> minMaxDiffs;

        for (double ang = 0; ang < createData.angleExtent; ang += angleStep)
        {
            const double deg = createData.angleStart + ang;
            SpamRgn originRgn(tmplRgns[l] * Geom::Translate(-anchorPoint.x, -anchorPoint.y)* Geom::Rotate::from_degrees(-deg), maskBuf);

            PointSet pointSetOrigin(originRgn);
            PointSet pointSetTmpl(originRgn, pyrImgCenter);

            cv::Mat rotMat = cv::getRotationMatrix2D(pyrImgCenter, deg, 1.0);
            cv::warpAffine(transPyrImg, transRotPyrImg, rotMat, cv::Size(transPyrImg.cols, transPyrImg.rows));

            if (!pointSetTmpl.IsInsideImage(cv::Size(transRotPyrImg.cols, transRotPyrImg.rows)))
            {
                return SpamResult::kSR_TM_TEMPL_REGION_OUT_OF_RANGE;
            }

            tmplDatas.emplace_back(static_cast<float>(deg), 1.f);
            PixelTemplData &ptd = tmplDatas.back();

            PointSet &pixlLocs = ptd.pixlLocs;
            const int numPoints = static_cast<int>(pointSetTmpl.size());
            for (int n=0; n<numPoints; ++n)
            {
                uint8_t minMaxDiff = 0;
                const cv::Point &tmplPt = pointSetTmpl[n];
                auto itF = minMaxDiffs.find(tmplPt.y * pyrImg.cols + tmplPt.x);
                if (itF != minMaxDiffs.end())
                {
                    minMaxDiff = itF->second;
                }
                else
                {
                    minMaxDiff = getMinMaxGrayScale(transRotPyrImg, maskPoints, tmplPt);
                    minMaxDiffs[tmplPt.y * pyrImg.cols + tmplPt.x] = minMaxDiff;
                }

                if (minMaxDiff > 10)
                {
                    pixlLocs.push_back(pointSetOrigin[n]);
                    ptd.pixlVals.push_back(transRotPyrImg.at<uint8_t>(pointSetTmpl[n]));
                }
            }

            if (pixlLocs.size()<3)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }

            const auto minMaxPoints = pixlLocs.MinMax();
            ptd.minPoint = minMaxPoints.first;
            ptd.maxPoint = minMaxPoints.second;

            AngleRange<double> angleRange{ deg , deg + angleStep };
            if (angleRange.between(0))
            {
                ang = -angleStep - createData.angleStart;
            }
        }
    }

    return SpamResult::kSR_OK;
}

SpamResult PixelTemplate::fastCreateTemplate(const PixelTmplCreateData &createData)
{
    if (createData.tmplRgn.empty())
    {
        return SpamResult::kSR_TM_EMPTY_TEMPL_REGION;
    }

    std::vector<Geom::PathVector> tmplRgns;
    tmplRgns.push_back(createData.tmplRgn);

    double s = 0.5;
    for (int l = 1; l < createData.pyramidLevel; ++l)
    {
        tmplRgns.push_back(createData.tmplRgn*Geom::Scale(s, s));
        s *= 0.5;
    }

    cv::buildPyramid(createData.srcImg, pyrs_, createData.pyramidLevel - 1, cv::BORDER_REFLECT);
    SpamRgn maskRgn(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(0, 0), 5))));
    PointSet maskPoints(maskRgn);
    std::vector<uint8_t> maskBuf;

    for (int l = 0; l < createData.pyramidLevel; ++l)
    {
        const SpamRgn rgn(tmplRgns[l], maskBuf);
        cv::Point anchorPoint{ cvRound(rgn.Centroid().x), cvRound(rgn.Centroid().y) };
        cfs_.emplace_back(static_cast<float>(anchorPoint.x), static_cast<float>(anchorPoint.y));

        Geom::Circle minCircle = rgn.MinCircle();
        if (minCircle.radius() < 5)
        {
            return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
        }

        const auto angleStep = Geom::deg_from_rad(std::acos(1 - 2 / (minCircle.radius()*minCircle.radius())));
        if (angleStep < 0.3)
        {
            return SpamResult::kSR_TM_TEMPL_REGION_TOO_LARGE;
        }

        pyramid_tmpl_datas_.emplace_back(static_cast<float>(angleStep), 0.f);
        LayerTemplData &ltd = pyramid_tmpl_datas_.back();

        const cv::Mat &pyrImg = pyrs_[l];
        ltd.tmplDatas = PixelTemplDatas();
        auto &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
        std::map<int, uint8_t> minMaxDiffs;

        for (double ang = 0; ang < createData.angleExtent; ang += angleStep)
        {
            const double deg = createData.angleStart + ang;
            const SpamRgn originRgn(tmplRgns[l] * Geom::Translate(-anchorPoint.x, -anchorPoint.y)* Geom::Rotate::from_degrees(-deg), maskBuf);

            PointSet pointSetOrigin(originRgn);
            PointSet pointSetTmpl(originRgn, anchorPoint);
            std::vector<cv::Point2f> tmplPoints;
            tmplPoints.reserve(pointSetTmpl.size());
            for (const cv::Point &pt : pointSetTmpl)
            {
                tmplPoints.emplace_back(static_cast<float>(pt.x), static_cast<float>(pt.y));
            }

            std::vector<cv::Point2f> tmplSrcPts;
            cv::Mat rotMat = cv::getRotationMatrix2D(anchorPoint, -deg, 1.0);
            cv::transform(tmplPoints, tmplSrcPts, rotMat);

            tmplDatas.emplace_back(static_cast<float>(deg), 1.f);
            PixelTemplData &ptd = tmplDatas.back();

            PointSet &pixlLocs = ptd.pixlLocs;
            const int numPoints = static_cast<int>(pointSetTmpl.size());
            for (int n = 0; n < numPoints; ++n)
            {
                const cv::Point2f &tmplSrcPt = tmplSrcPts[n];
                const cv::Point tmplSrcPti(tmplSrcPt);

                uint8_t minMaxDiff = 0;
                auto itF = minMaxDiffs.find(tmplSrcPti.y * pyrImg.cols + tmplSrcPti.x);
                if (itF != minMaxDiffs.end())
                {
                    minMaxDiff = itF->second;
                }
                else
                {
                    minMaxDiff = getMinMaxGrayScale(pyrImg, maskPoints, tmplSrcPti);
                    minMaxDiffs[tmplSrcPti.y * pyrImg.cols + tmplSrcPti.x] = minMaxDiff;
                }

                if (minMaxDiff > 10)
                {
                    int16_t grayVal = PixelTemplate::getGrayScaleSubpix(pyrImg, tmplSrcPt);
                    if (grayVal >= 0)
                    {
                        pixlLocs.push_back(pointSetOrigin[n]);
                        ptd.pixlVals.push_back(grayVal);
                    }
                }
            }

            if (pixlLocs.size() < 3)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }

            const auto minMaxPoints = pixlLocs.MinMax();
            ptd.minPoint = minMaxPoints.first;
            ptd.maxPoint = minMaxPoints.second;

            AngleRange<double> angleRange{ deg , deg + angleStep };
            if (angleRange.between(0))
            {
                ang = -angleStep - createData.angleStart;
            }
        }
    }

    return SpamResult::kSR_OK;
}

void PixelTemplate::linkTemplatesBetweenLayers()
{
    const int topLayerIndex = static_cast<int>(pyramid_tmpl_datas_.size()-1);
    for (int layer = topLayerIndex; layer > 0; --layer)
    {
        LayerTemplData &ltd = pyramid_tmpl_datas_[layer];
        LayerTemplData &belowLtd = pyramid_tmpl_datas_[layer-1];
        auto &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
        const auto &belowTmplDatas = boost::get<PixelTemplDatas>(belowLtd.tmplDatas);
        for (PixelTemplData &ptd : tmplDatas)
        {
            AngleRange<float> angleRange(ptd.angle - ltd.angleStep, ptd.angle + ltd.angleStep);
            for (int t=0; t< belowTmplDatas.size(); ++t)
            {
                if (angleRange.contains(belowTmplDatas[t].angle))
                {
                    ptd.mindices.push_back(t);
                }
            }
        }
    }
}

uint8_t PixelTemplate::getMinMaxGrayScale(const cv::Mat &img, const PointSet &maskPoints, const cv::Point &point)
{
    cv::Rect imageBox(0, 0, img.cols, img.rows);
    uint8_t minGrayScale = std::numeric_limits<uint8_t>::max();
    uint8_t maxGrayScale = std::numeric_limits<uint8_t>::min();
    for (const cv::Point &maskPoint : maskPoints)
    {
        cv::Point pixelPoint = point + maskPoint;
        if (imageBox.contains(pixelPoint))
        {
            const uint8_t grayScale = img.at<uint8_t>(pixelPoint);
            if (grayScale < minGrayScale)
            {
                minGrayScale = grayScale;
            }

            if (grayScale > maxGrayScale)
            {
                maxGrayScale = grayScale;
            }
        }
    }

    return (minGrayScale < maxGrayScale) ? (maxGrayScale - minGrayScale) : 0;
}

void PixelTemplate::supressNoneMaximum()
{
    candidate_runs_.RLEncodeCandidates(candidates_);
    candidate_runs_.Connect(candidate_groups_);
    candidates_.resize(candidate_groups_.size());

    Candidate *candidates = candidates_.data();
    for (const CandidateGroup &cg : candidate_groups_)
    {
        *candidates = cg.front().best;
        for (const CandidateRun &cr : cg)
        {
            if (cr.best.score > candidates->score)
            {
                *candidates = cr.best;
            }
        }
        candidates += 1;
    }

    top_candidates_.assign(candidates_.cbegin(), candidates_.cend());
}

SpamResult PixelTemplate::changeToNCCTemplate()
{
    for (LayerTemplData &ltd : pyramid_tmpl_datas_)
    {
        NCCTemplDatas ntds;
        PixelTemplDatas &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
        for (PixelTemplData &ptd : tmplDatas)
        {
            ntds.emplace_back(ptd.angle, ptd.scale);
            NCCTemplData &ntd = ntds.back();
            ntd.minPoint = ptd.minPoint;
            ntd.maxPoint = ptd.maxPoint;
            ntd.mindices = ptd.mindices;

            const int numTotalPoints = static_cast<int>(ptd.pixlLocs.size());
            int partitionIndex = static_cast<int>(ptd.pixlLocs.size()*0.2);

            if (partitionIndex<1)
            {
                return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
            }

            for (int n=partitionIndex; n<numTotalPoints; ++n)
            {
                if (ptd.pixlLocs[n].y != ptd.pixlLocs[n-1].y)
                {
                    partitionIndex = n;
                    break;
                }
            }

            if (partitionIndex==(numTotalPoints-1))
            {
                return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
            }

            ntd.partASum = 0;
            ntd.partBSum = 0;
            ntd.partASqrSum = 0;
            ntd.partBSqrSum = 0;
            ntd.cPartABoundaries = 0;
            for (int n=0; n<partitionIndex; ++n)
            {
                bool backPoint = (0 == n || (ptd.pixlLocs[n].y != ptd.pixlLocs[n - 1].y) || (ptd.pixlLocs[n].x != (ptd.pixlLocs[n - 1].x + 1)));
                bool frontPoint = ((partitionIndex - 1) == n || (ptd.pixlLocs[n].y != ptd.pixlLocs[n + 1].y) || (ptd.pixlLocs[n].x != (ptd.pixlLocs[n + 1].x - 1)));

                if (backPoint && !frontPoint)
                {
                    ntd.partALocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveBack);
                    ntd.partAVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.cPartABoundaries += 1;
                    ntd.partASum += ptd.pixlVals[n];
                    ntd.partASqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }

                if (frontPoint && !backPoint)
                {
                    ntd.partALocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveFront);
                    ntd.partAVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.partASum += ptd.pixlVals[n];
                    ntd.partASqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }

                if (!frontPoint && !backPoint)
                {
                    ntd.partALocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveMiddle);
                    ntd.partAVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.partASum += ptd.pixlVals[n];
                    ntd.partASqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }
            }

            if (ntd.partALocs.size() < 3)
            {
                return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
            }

            for (int n = partitionIndex; n < numTotalPoints; ++n)
            {
                bool backPoint = (0 == n || (ptd.pixlLocs[n].y != ptd.pixlLocs[n - 1].y) || (ptd.pixlLocs[n].x != (ptd.pixlLocs[n - 1].x + 1)));
                bool frontPoint = ((numTotalPoints - 1) == n || (ptd.pixlLocs[n].y != ptd.pixlLocs[n + 1].y) || (ptd.pixlLocs[n].x != (ptd.pixlLocs[n + 1].x - 1)));

                if (backPoint && !frontPoint)
                {
                    ntd.partBLocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveBack);
                    ntd.partBVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.partBBoundaries.emplace_back(ptd.pixlLocs[n], cv::Point());
                    ntd.partBSum += ptd.pixlVals[n];
                    ntd.partBSqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }

                if (frontPoint && !backPoint)
                {
                    ntd.partBLocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveFront);
                    ntd.partBVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.partBBoundaries.back().second = ptd.pixlLocs[n];
                    ntd.partBSum += ptd.pixlVals[n];
                    ntd.partBSqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }

                if (!frontPoint && !backPoint)
                {
                    ntd.partBLocs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y, kTP_WaveMiddle);
                    ntd.partBVals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                    ntd.partBSum += ptd.pixlVals[n];
                    ntd.partBSqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
                }
            }

            if (ntd.partBLocs.size() < 3)
            {
                return SpamResult::kSR_TM_TEMPL_REGION_TOO_SMALL;
            }

            int64_t numPoints = static_cast<int64_t>(ntd.partALocs.size() + ntd.partBLocs.size());
            int64_t T = ntd.partASqrSum + ntd.partBSqrSum;
            int64_t S = ntd.partASum + ntd.partBSum;
            ntd.norm = calcValue1(T, S, numPoints);
            if (ntd.norm < 0)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }

            int64_t numPartB = static_cast<int64_t>(ntd.partBLocs.size());
            ntd.betaz = calcValue2(ntd.partBSqrSum, S, ntd.partBSum, numPoints, numPartB);
            if (ntd.betaz < 0)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }
        }

        ltd.tmplDatas = ntds;
    }

    return SpamResult::kSR_SUCCESS;
}

SpamResult PixelTemplate::changeToBruteForceNCCTemplate()
{
    for (LayerTemplData &ltd : pyramid_tmpl_datas_)
    {
        BFNCCTemplDatas ntds;
        PixelTemplDatas &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
        for (PixelTemplData &ptd : tmplDatas)
        {
            ntds.emplace_back(ptd.angle, ptd.scale);
            BruteForceNCCTemplData &ntd = ntds.back();
            ntd.minPoint = ptd.minPoint;
            ntd.maxPoint = ptd.maxPoint;
            ntd.mindices = ptd.mindices;

            const int numTotalPoints = static_cast<int>(ptd.pixlLocs.size());

            ntd.sum = 0;
            ntd.sqrSum = 0;
            for (int n = 0; n < numTotalPoints; ++n)
            {
                ntd.locs.emplace_back(ptd.pixlLocs[n].x, ptd.pixlLocs[n].y);
                ntd.vals.push_back(static_cast<uint8_t>(ptd.pixlVals[n]));
                ntd.sum += ptd.pixlVals[n];
                ntd.sqrSum += (ptd.pixlVals[n] * ptd.pixlVals[n]);
            }

            constexpr int simdSize = 8;
            ntd.regVals.resize(0);
            ntd.regVals.resize((static_cast<int>(ntd.vals.size()) + simdSize - 1) & (-simdSize), 0);
            uint32_t *pTmplVals = ntd.regVals.data();
            for (uint8_t v : ntd.vals)
            {
                *pTmplVals = v; pTmplVals += 1;
            }

            int64_t numPoints = numTotalPoints;
            int64_t T = ntd.sqrSum;
            int64_t S = ntd.sum;
            ntd.norm = calcValue1(T, S, numPoints);
            if (ntd.norm < 0)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }
        }

        ltd.tmplDatas = ntds;
    }

    return SpamResult::kSR_SUCCESS;
}

void PixelTemplate::processToplayerSearchROI(const PixelTmplCreateData &createData)
{
    if (createData.roi.empty())
    {
        top_layer_search_roi_.clear();
        top_layer_search_roi_g_.clear();
    }
    else
    {
        double s = std::pow(0.5, createData.pyramidLevel-1);
        top_layer_search_roi_g_ = createData.roi*Geom::Scale(s, s);
        top_layer_search_roi_.SetRegion(top_layer_search_roi_g_, std::vector<uint8_t>());
    }
}

cv::Mat PixelTemplate::GetTopScoreMat() const
{
    cv::Mat scoreMat(pyrs_.back().rows, pyrs_.back().cols, CV_8UC1, cv::Scalar());
    if (cv::TM_CCOEFF_NORMED== match_mode_ ||
        cv::TM_CCOEFF == match_mode_)
    {
        for (const auto &candidate : top_candidates_)
        {
            scoreMat.at<uint8_t>(cv::Point(candidate.col, candidate.row)) = cv::saturate_cast<uint8_t>((candidate.score+1) / 2 * 255);
        }
    }
    else
    {
        for (const auto &candidate : top_candidates_)
        {
            scoreMat.at<uint8_t>(cv::Point(candidate.col, candidate.row)) = static_cast<uint8_t>(candidate.score);
        }
    }

    return scoreMat;
}