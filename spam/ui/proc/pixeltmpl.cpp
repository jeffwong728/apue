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
    SADTopLayerScaner(const PixelTemplate *const pixTmpl, const int s) : tmpl(pixTmpl), sad(s) {}
    SADTopLayerScaner(SADTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), sad(s.sad) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(SADTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }

    const int sad;
    const PixelTemplate *const tmpl;
    PixelTemplate::CandidateList candidates;
};

void SADTopLayerScaner::operator()(const tbb::blocked_range<int>& br)
{
    const cv::Mat &topLayer = tmpl->pyrs_.back();
    const int nCols = topLayer.cols;
    constexpr int32_t simdSize = 16;
    std::array<int16_t, simdSize> tempData;
    const LayerTmplData &topLayerTmplData = tmpl->pyramid_tmpl_datas_.back();
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
            for (int t = 0; t < static_cast<int>(topLayerTmplData.tmplDatas.size()); ++t)
            {
                const PixelTmplData &ptd = topLayerTmplData.tmplDatas[t];

                if (oib(originPt + ptd.minPoint) || oib(originPt + ptd.maxPoint))
                {
                    continue;
                }

                const int32_t numPoints = static_cast<int32_t>(ptd.pixlLocs.size());
                const int32_t regularSize = numPoints & (-simdSize);
                const auto &pixlVals = boost::get<std::vector<int16_t>>(ptd.pixlVals);
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

SpamResult PixelTemplate::matchTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle)
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

    SADTopLayerScaner sadScaner(this, sad);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, nTopRows), sadScaner);
    candidates_.swap(sadScaner.candidates);
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
        LayerTmplData &ltd = pyramid_tmpl_datas_[layer];
        const cv::Mat &layerMat = pyrs_[layer];
        const int nLayerCols = layerMat.cols;
        const int nLayerRows = layerMat.rows;
        constexpr int32_t simdSize = 16;
        std::array<int16_t, simdSize> tempData;
        const LayerTmplData &layerTmplData = pyramid_tmpl_datas_[layer];
        OutsideImageBox oib(layerMat.cols, layerMat.rows);
        const int layerSad = sad + 5 * layer;

        row_ptrs_.clear();
        row_ptrs_.resize(nLayerRows);
        for (int row = 0; row < nLayerRows; ++row)
        {
            row_ptrs_[row] = layerMat.ptr<uint8_t>(row);
        }

        for (Candidate &candidate : final_candidates_)
        {
            const int row = candidate.row;
            const int col = candidate.col;
            cv::Point originPt{ col, row };
            int32_t minSAD = std::numeric_limits<int32_t>::max();
            int bestTmplIndex = 0;
            const std::vector<int> &tmplIndices = pyramid_tmpl_datas_[layer + 1].tmplDatas[candidate.mindex].belowCandidates;
            for (const int tmplIndex : tmplIndices)
            {
                const PixelTmplData &ptd = ltd.tmplDatas[tmplIndex];

                if (oib(originPt + ptd.minPoint) || oib(originPt + ptd.maxPoint))
                {
                    continue;
                }

                const int32_t numPoints = static_cast<int32_t>(ptd.pixlLocs.size());
                const int32_t regularSize = numPoints & (-simdSize);
                const auto &pixlVals = boost::get<std::vector<int16_t>>(ptd.pixlVals);
                const cv::Point *pPixlLocs = ptd.pixlLocs.data();

                int32_t n = 0;
                int32_t partialSum = 0;
                for (; n < regularSize; n += simdSize)
                {
                    for (int m = 0; m < simdSize; ++m)
                    {
                        const int x = pPixlLocs->x + col;
                        const int y = pPixlLocs->y + row;
                        tempData[m] = row_ptrs_[y][x];
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
                    partialSum += std::abs(pixlVals[n] - row_ptrs_[y][x]);
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
        angle = pyramid_tmpl_datas_[0].tmplDatas[bestCandidate.mindex].angle;

        return SpamResult::kSR_OK;
    }
    else
    {
        return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
    }
}

SpamResult PixelTemplate::CreatePixelTemplate(const PixelTmplCreateData &createData)
{
    destroyData();
    SpamResult sr = verifyCreateData(createData);
    if (SpamResult::kSR_OK != sr)
    {
        destroyData();
        return sr;
    }

    sr = calcCentreOfGravity(createData);
    if (SpamResult::kSR_OK != sr)
    {
        destroyData();
        return sr;
    }

    linkTemplatesBetweenLayers();

    pyramid_level_ = createData.pyramidLevel;
    match_mode_    = createData.matchMode;

    return SpamResult::kSR_OK;
}

void PixelTemplate::destroyData()
{
    pyramid_level_ = 1;
    match_mode_ = cv::TM_SQDIFF;
    cfs_.clear();
    tmpl_rgns_.clear();
    search_rois_.clear();
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

SpamResult PixelTemplate::calcCentreOfGravity(const PixelTmplCreateData &createData)
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
    cv::medianBlur(createData.srcImg, tmplImg, 5);
    cv::buildPyramid(tmplImg, pyrs_, createData.pyramidLevel-1, cv::BORDER_REFLECT);
    SpamRgn maskRgn(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(0, 0), 5))));
    PointSet maskPoints(maskRgn);

    cv::Mat transPyrImg;
    cv::Mat transRotPyrImg;
    cv::Mat transMat = (cv::Mat_<double>(2, 3) << 1, 0, 0, 0, 1, 0);

    for (int l=0; l < createData.pyramidLevel; ++l)
    {
        const SpamRgn rgn(tmplRgns[l]);
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
        LayerTmplData &ltl = pyramid_tmpl_datas_.back();

        const cv::Mat &pyrImg = pyrs_[l];
        const cv::Point &pyrImgCenter{ pyrImg.cols / 2, pyrImg.rows / 2 };

        transMat.at<double>(0, 2) = pyrImgCenter.x - anchorPoint.x;
        transMat.at<double>(1, 2) = pyrImgCenter.y - anchorPoint.y;
        cv::warpAffine(pyrImg, transPyrImg, transMat, cv::Size(pyrImg.cols, pyrImg.rows));

        for (double ang = 0; ang < createData.angleExtent; ang += angleStep)
        {
            const double deg = createData.angleStart + ang;
            SpamRgn originRgn(tmplRgns[l] * Geom::Translate(-anchorPoint.x, -anchorPoint.y)* Geom::Rotate::from_degrees(-deg));

            PointSet pointSetOrigin(originRgn);
            PointSet pointSetTmpl(originRgn, pyrImgCenter);

            cv::Mat rotMat = cv::getRotationMatrix2D(pyrImgCenter, deg, 1.0);
            cv::warpAffine(transPyrImg, transRotPyrImg, rotMat, cv::Size(transPyrImg.cols, transPyrImg.rows));

            if (!pointSetTmpl.IsInsideImage(cv::Size(transRotPyrImg.cols, transRotPyrImg.rows)))
            {
                return SpamResult::kSR_TM_TEMPL_REGION_OUT_OF_RANGE;
            }

            ltl.tmplDatas.emplace_back(static_cast<float>(deg), 1.f);
            PixelTmplData &ptd = ltl.tmplDatas.back();

            ptd.pixlVals = std::vector<int16_t>();
            PointSet &pixlLocs = ptd.pixlLocs;
            const int numPoints = static_cast<int>(pointSetTmpl.size());
            for (int n=0; n<numPoints; ++n)
            {
                if (getMinMaxGrayScale(transRotPyrImg, maskPoints, pointSetTmpl[n]) > 10)
                {
                    pixlLocs.push_back(pointSetOrigin[n]);
                    boost::get<std::vector<int16_t>>(ptd.pixlVals).push_back(transRotPyrImg.at<uint8_t>(pointSetTmpl[n]));
                }
            }

            if (pixlLocs.size()<3)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }

            const auto minMaxPoints = pixlLocs.MinMax();
            ptd.minPoint = minMaxPoints.first;
            ptd.maxPoint = minMaxPoints.second;
        }
    }

    return SpamResult::kSR_OK;
}

void PixelTemplate::linkTemplatesBetweenLayers()
{
    const int topLayerIndex = static_cast<int>(pyramid_tmpl_datas_.size()-1);
    for (int layer = topLayerIndex; layer > 0; --layer)
    {
        LayerTmplData &ltd = pyramid_tmpl_datas_[layer];
        LayerTmplData &belowLtd = pyramid_tmpl_datas_[layer-1];
        for (PixelTmplData &ptd : ltd.tmplDatas)
        {
            AngleRange angleRange(ptd.angle - ltd.angleStep, ptd.angle + ltd.angleStep);
            for (int t=0; t< belowLtd.tmplDatas.size(); ++t)
            {
                if (angleRange.contains(belowLtd.tmplDatas[t].angle))
                {
                    ptd.belowCandidates.push_back(t);
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

cv::Mat PixelTemplate::GetTopScoreMat() const
{
    cv::Mat scoreMat(pyrs_.back().rows, pyrs_.back().cols, CV_8UC1, cv::Scalar());
    for (const auto &candidate : top_candidates_)
    {
        scoreMat.at<uint8_t>(cv::Point(candidate.col, candidate.row)) = static_cast<uint8_t>(candidate.score);
    }

    return scoreMat;
}