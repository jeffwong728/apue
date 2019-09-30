#include "shapetmpl.h"
#include "gradient.h"
#include "basic.h"
#include <iomanip>
#include <limits>
#include <stack>
#include <numeric>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244 )
#include <2geom/path-sink.h>
#include <2geom/path-intersection.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

template<bool TouchBorder>
struct ShapeTopLayerScaner
{
    using GradientList = std::vector<float, tbb::scalable_allocator<float>>;
    using CandidateDict = std::map<int, BaseTemplate::Candidate, std::less<int>, tbb::scalable_allocator<std::pair<const int, BaseTemplate::Candidate>>>;
    ShapeTopLayerScaner(const ShapeTemplate *const shapeTmpl, const SpamRun *const r, const float s, const float g)
        : tmpl(shapeTmpl)
        , roi(r)
        , minScore(s)
        , greediness(g)
    {}

    ShapeTopLayerScaner(ShapeTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), roi(s.roi), minScore(s.minScore), greediness(s.greediness) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(ShapeTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }
    void copyCandidates(const CandidateDict &aCandidates)
    {
        candidates.reserve(aCandidates.size());
        for (const auto &cItem : aCandidates)
        {
            candidates.push_back(cItem.second);
        }
    }

    static float dotProduct(const float *xVec0, const float *yVec0, const float *xVec1, const float *yVec1)
    {
        vcl::Vec8f xvec0, yvec0, xvec1, yvec1;
        xvec0.load(xVec0);
        yvec0.load(yVec0);
        xvec1.load(xVec1);
        yvec1.load(yVec1);
        vcl::Vec8f dot = xvec0 * xvec1 + yvec0 * yvec1;
        return vcl::horizontal_add(dot);
    }

    static void tryRecordCandidate(const float score, const float layerMinScore, const int row, const int col, const int t, const int nLayerCols, CandidateDict &aCandidates)
    {
        if (score >= layerMinScore)
        {
            BaseTemplate::Candidate candidate{ row, col, t, static_cast<float>(score) };
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

    const float minScore;
    const float greediness;
    const ShapeTemplate *const tmpl;
    const SpamRun *const roi;
    BaseTemplate::CandidateList candidates;
};

template<bool TouchBorder>
struct ShapeCandidateScaner
{
    ShapeCandidateScaner(ShapeTemplate *const shapeTmpl, const float s, const float g, const int l, const int mc)
        : tmpl(shapeTmpl)
        , minScore(s)
        , greediness(g)
        , layer(l)
        , minContrast(mc)
        , ones8i(1)
        , zeros8i(0)
    {}

    void operator()(const tbb::blocked_range<int>& r) const;
    float getScore(const int row, const int col, const int ang) const;

    vcl::Vec8i kernel(const vcl::Vec8i(&pixelVals)[9], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1) const
    {
        return (pixelVals[i1] - pixelVals[i0]) + 2 * (pixelVals[j1] - pixelVals[j0]) + (pixelVals[k1] - pixelVals[k0]);
    }

    void normalize(const vcl::Vec8i &X, const vcl::Vec8i &Y, const vcl::Vec8i &minContrast, float *xNorm, float *yNorm) const
    {
        vcl::Vec8i sqrSum = X * X + Y * Y;
        vcl::Vec8i normSqrSum = vcl::select(sqrSum == 0, ones8i, sqrSum);
        vcl::Vec8i supress = sqrSum < minContrast;
        vcl::Vec8i supressX = vcl::select(supress, zeros8i, X);
        vcl::Vec8i supressY = vcl::select(supress, zeros8i, Y);
        vcl::Vec8f rSqrt = vcl::approx_rsqrt(vcl::to_float(normSqrSum));
        vcl::Vec8f NX = vcl::to_float(supressX) * rSqrt;
        vcl::Vec8f NY = vcl::to_float(supressY) * rSqrt;
        NX.store(xNorm);
        NY.store(yNorm);
    }

    void fillSobelNeigborhood(const cv::Point *&pEdgePt, const int row, const int col, const int i, int32_t(&partVals)[9][8]) const
    {
        const int x = pEdgePt->x + col, y = pEdgePt->y + row;
        partVals[0][i] = tmpl->row_ptrs_[y - 1][x - 1];
        partVals[1][i] = tmpl->row_ptrs_[y - 1][x];
        partVals[2][i] = tmpl->row_ptrs_[y - 1][x + 1];
        partVals[3][i] = tmpl->row_ptrs_[y][x - 1];
        partVals[5][i] = tmpl->row_ptrs_[y][x + 1];
        partVals[6][i] = tmpl->row_ptrs_[y + 1][x - 1];
        partVals[7][i] = tmpl->row_ptrs_[y + 1][x];
        partVals[8][i] = tmpl->row_ptrs_[y + 1][x + 1];
        pEdgePt += 1;
    }

    void fillSobelNeigborhood(const cv::Point (&edgePts)[8], const int row, const int col, const int i, int32_t(&partVals)[9][8]) const
    {
        const int x = edgePts[i].x + col, y = edgePts[i].y + row;
        partVals[0][i] = tmpl->row_ptrs_[y - 1][x - 1];
        partVals[1][i] = tmpl->row_ptrs_[y - 1][x];
        partVals[2][i] = tmpl->row_ptrs_[y - 1][x + 1];
        partVals[3][i] = tmpl->row_ptrs_[y][x - 1];
        partVals[5][i] = tmpl->row_ptrs_[y][x + 1];
        partVals[6][i] = tmpl->row_ptrs_[y + 1][x - 1];
        partVals[7][i] = tmpl->row_ptrs_[y + 1][x];
        partVals[8][i] = tmpl->row_ptrs_[y + 1][x + 1];
    }

    void fillSobelNeigborhood(const cv::Point *&pEdgePt, const cv::Point &ep, const int i, int32_t(&partVals)[9][8]) const
    {
        partVals[0][i] = tmpl->row_ptrs_[ep.y - 1][ep.x - 1];
        partVals[1][i] = tmpl->row_ptrs_[ep.y - 1][ep.x];
        partVals[2][i] = tmpl->row_ptrs_[ep.y - 1][ep.x + 1];
        partVals[3][i] = tmpl->row_ptrs_[ep.y][ep.x - 1];
        partVals[5][i] = tmpl->row_ptrs_[ep.y][ep.x + 1];
        partVals[6][i] = tmpl->row_ptrs_[ep.y + 1][ep.x - 1];
        partVals[7][i] = tmpl->row_ptrs_[ep.y + 1][ep.x];
        partVals[8][i] = tmpl->row_ptrs_[ep.y + 1][ep.x + 1];
        pEdgePt += 1;
    }

    void fillSobelNeigborhood(const cv::Point *&pEdgePt, const int i, int32_t(&partVals)[9][8]) const
    {
        partVals[0][i] = 0;
        partVals[1][i] = 0;
        partVals[2][i] = 0;
        partVals[3][i] = 0;
        partVals[5][i] = 0;
        partVals[6][i] = 0;
        partVals[7][i] = 0;
        partVals[8][i] = 0;
        pEdgePt += 1;
    }

    void fillSobelNeigborhood(const int i, int32_t(&partVals)[9][8]) const
    {
        partVals[0][i] = 0;
        partVals[1][i] = 0;
        partVals[2][i] = 0;
        partVals[3][i] = 0;
        partVals[5][i] = 0;
        partVals[6][i] = 0;
        partVals[7][i] = 0;
        partVals[8][i] = 0;
    }

    static void deformClusterEdgePoints(const cv::Point *pEdgePt, const cv::Point &deform, cv::Point (&edgePts)[8])
    {
        edgePts[0] = pEdgePt[0] + deform;
        edgePts[1] = pEdgePt[1] + deform;
        edgePts[2] = pEdgePt[2] + deform;
        edgePts[3] = pEdgePt[3] + deform;
        edgePts[4] = pEdgePt[4] + deform;
        edgePts[5] = pEdgePt[5] + deform;
        edgePts[6] = pEdgePt[6] + deform;
        edgePts[7] = pEdgePt[7] + deform;
    }

    float getPartScore(const int32_t (&partVals)[9][8], const vcl::Vec8i &vecMinContrast, const float *tmplDx, const float *tmplDy) const
    {
        vcl::Vec8i pixelVals[9];
        pixelVals[0].load(partVals[0]); pixelVals[1].load(partVals[1]); pixelVals[2].load(partVals[2]);
        pixelVals[3].load(partVals[3]); pixelVals[5].load(partVals[5]);
        pixelVals[6].load(partVals[6]); pixelVals[7].load(partVals[7]); pixelVals[8].load(partVals[8]);

        std::array<float, 8> partDXVals, partDYVals;
        vcl::Vec8i X = kernel(pixelVals, 0, 2, 3, 5, 6, 8);
        vcl::Vec8i Y = kernel(pixelVals, 0, 6, 1, 7, 2, 8);
        normalize(X, Y, vecMinContrast, partDXVals.data(), partDYVals.data());

        return ShapeTopLayerScaner<false>::dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
    }

    const float minScore;
    const float greediness;
    const int layer;
    const int minContrast;
    ShapeTemplate *const tmpl;
    const vcl::Vec8i ones8i;
    const vcl::Vec8i zeros8i;
};

template<bool TouchBorder>
struct DeformCandidateScaner : public ShapeCandidateScaner<TouchBorder>
{
    DeformCandidateScaner(ShapeTemplate *const shapeTmpl, const float s, const float g, const int l, const int mc)
        : ShapeCandidateScaner<TouchBorder>(shapeTmpl, s, g, l, mc)
    {}

    void operator()(const tbb::blocked_range<int>& r) const;
};

template<>
void ShapeTopLayerScaner<false>::operator()(const tbb::blocked_range<int>& br)
{
    constexpr int simdSize = 8;
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerShapeData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * (tmpl->pyramid_level_ - 1));
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const int runStart = br.begin();
    const int runEnd = br.end();
    const std::vector<ShapeTemplData> &tmplDatas = layerTempls.tmplDatas;
    const int numLayerTempls = static_cast<int>(tmplDatas.size());
    CandidateDict aCandidates;

    for (int t = 0; t < numLayerTempls; ++t)
    {
        const ShapeTemplData &ntd = tmplDatas[t];
        const int numEdges = static_cast<int>(ntd.edgeLocs.size());
        const int regularNumEdges = numEdges & (-simdSize);
        const float stopScore = numEdges * layerMinScore - numEdges;

        for (int run = runStart; run < runEnd; ++run)
        {
            const int row = roi[run].row;
            const int colStart = roi[run].colb;
            const int colEnd = roi[run].cole;

            for (int col = colStart; col < colEnd; ++col)
            {
                int e = 0, j = 0;
                const float *tmplDx = ntd.gNXVals.data();
                const float *tmplDy = ntd.gNYVals.data();
                const cv::Point *pPt = ntd.edgeLocs.data();
                float sumDot = 0.f;
                const cv::Point anchorPt{ col, row };
                std::array<float, simdSize> partDXVals, partDYVals;

                if (oib(anchorPt + ntd.minPoint) || oib(anchorPt + ntd.maxPoint))
                {
                    continue;
                }

                for (; e < regularNumEdges; e += simdSize)
                {
                    for (int i = 0; i < simdSize; ++i)
                    {
                        const int x = pPt->x + col, y = pPt->y + row;
                        partDXVals[i] = tmpl->dx_row_ptrs_[y][x];
                        partDYVals[i] = tmpl->dy_row_ptrs_[y][x];
                        pPt += 1;
                    }

                    j += simdSize;
                    sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                    float safeScore = stopScore + j * f;
                    float greedyScore = layerMinScore * j;
                    if (sumDot < std::min(safeScore, greedyScore)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                }

                if (e < regularNumEdges) { continue; }

                if (e < numEdges)
                {
                    std::memset(partDXVals.data(), 0, partDXVals.size() * sizeof(partDXVals[0]));
                    std::memset(partDYVals.data(), 0, partDYVals.size() * sizeof(partDYVals[0]));
                    for (int k = 0; e < numEdges; ++e, ++k)
                    {
                        const int x = pPt->x + col, y = pPt->y + row;
                        partDXVals[k] = tmpl->dx_row_ptrs_[y][x];
                        partDYVals[k] = tmpl->dy_row_ptrs_[y][x];

                        pPt += 1;
                    }

                    sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                }

                tryRecordCandidate(sumDot / numEdges, layerMinScore, row, col, t, nLayerCols, aCandidates);
            }
        }
    }

    copyCandidates(aCandidates);
}

template<>
void ShapeTopLayerScaner<true>::operator()(const tbb::blocked_range<int>& br)
{
    constexpr int simdSize = 8;
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerShapeData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * (tmpl->pyramid_level_ - 1));
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const int runStart = br.begin();
    const int runEnd = br.end();
    const std::vector<ShapeTemplData> &tmplDatas = layerTempls.tmplDatas;
    const int numLayerTempls = static_cast<int>(tmplDatas.size());
    CandidateDict aCandidates;

    for (int t = 0; t < numLayerTempls; ++t)
    {
        const ShapeTemplData &ntd = tmplDatas[t];
        const int numEdges = static_cast<int>(ntd.edgeLocs.size());
        const int regularNumEdges = numEdges & (-simdSize);
        const float stopScore = numEdges * layerMinScore - numEdges;

        for (int run = runStart; run < runEnd; ++run)
        {
            const int row = roi[run].row;
            const int colStart = roi[run].colb;
            const int colEnd = roi[run].cole;

            for (int col = colStart; col < colEnd; ++col)
            {
                int e = 0, j = 0;
                const float *tmplDx = ntd.gNXVals.data();
                const float *tmplDy = ntd.gNYVals.data();
                const cv::Point *pPt = ntd.edgeLocs.data();
                float sumDot = 0.f;
                const cv::Point anchorPt{ col, row };
                std::array<float, simdSize> partDXVals, partDYVals;

                if (oib(anchorPt + ntd.minPoint) || oib(anchorPt + ntd.maxPoint))
                {
                    for (; e < regularNumEdges; e += simdSize)
                    {
                        for (int i = 0; i < simdSize; ++i)
                        {
                            const cv::Point tPt{ pPt->x + col, pPt->y + row };
                            if (oib(tPt))
                            {
                                partDXVals[i] = 0.f;
                                partDYVals[i] = 0.f;
                            }
                            else
                            {
                                partDXVals[i] = tmpl->dx_row_ptrs_[tPt.y][tPt.x];
                                partDYVals[i] = tmpl->dy_row_ptrs_[tPt.y][tPt.x];
                            }

                            pPt += 1;
                        }

                        j += simdSize;
                        sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                        float safeScore = stopScore + j * f;
                        float greedyScore = layerMinScore * j;
                        if (sumDot < std::min(safeScore, greedyScore)) { break; }

                        tmplDx += simdSize;
                        tmplDy += simdSize;
                    }

                    if (e < regularNumEdges) { continue; }

                    if (e < numEdges)
                    {
                        std::memset(partDXVals.data(), 0, partDXVals.size() * sizeof(partDXVals[0]));
                        std::memset(partDYVals.data(), 0, partDYVals.size() * sizeof(partDYVals[0]));
                        for (int k = 0; e < numEdges; ++e, ++k)
                        {
                            const cv::Point tPt{ pPt->x + col, pPt->y + row };
                            if (!oib(tPt))
                            {
                                partDXVals[k] = tmpl->dx_row_ptrs_[tPt.y][tPt.x];
                                partDYVals[k] = tmpl->dy_row_ptrs_[tPt.y][tPt.x];
                            }

                            pPt += 1;
                        }

                        sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                    }
                }
                else
                {
                    for (; e < regularNumEdges; e += simdSize)
                    {
                        for (int i = 0; i < simdSize; ++i)
                        {
                            const int x = pPt->x + col, y = pPt->y + row;
                            partDXVals[i] = tmpl->dx_row_ptrs_[y][x];
                            partDYVals[i] = tmpl->dy_row_ptrs_[y][x];
                            pPt += 1;
                        }

                        j += simdSize;
                        sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                        float safeScore = stopScore + j * f;
                        float greedyScore = layerMinScore * j;
                        if (sumDot < std::min(safeScore, greedyScore)) { break; }

                        tmplDx += simdSize;
                        tmplDy += simdSize;
                    }

                    if (e < regularNumEdges) { continue; }

                    if (e < numEdges)
                    {
                        std::memset(partDXVals.data(), 0, partDXVals.size() * sizeof(partDXVals[0]));
                        std::memset(partDYVals.data(), 0, partDYVals.size() * sizeof(partDYVals[0]));
                        for (int k = 0; e < numEdges; ++e, ++k)
                        {
                            const int x = pPt->x + col, y = pPt->y + row;
                            partDXVals[k] = tmpl->dx_row_ptrs_[y][x];
                            partDYVals[k] = tmpl->dy_row_ptrs_[y][x];

                            pPt += 1;
                        }

                        sumDot += dotProduct(tmplDx, tmplDy, partDXVals.data(), partDYVals.data());
                    }
                }

                tryRecordCandidate(sumDot / numEdges, layerMinScore, row, col, t, nLayerCols, aCandidates);
            }
        }
    }

    copyCandidates(aCandidates);
}

template<>
void ShapeCandidateScaner<false>::operator()(const tbb::blocked_range<int>& r) const
{
    constexpr int simdSize = 8;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle orb(1, layerMat.cols-2, 1, layerMat.rows-2);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * layer);
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const auto &tmplDatas = ltd.tmplDatas;
    const auto &upperTmplDatas = tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);

    for (int c = r.begin(); c != r.end(); ++c)
    {
        BaseTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        const cv::Point anchorPt{ col, row };
        float maxScore = -1.f;
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;

        for (const int tmplIndex : tmplIndices)
        {
            const ShapeTemplData &ntd = tmplDatas[tmplIndex];
            const int numEdges = static_cast<int>(ntd.edgeLocs.size());
            const int regularNumEdges = numEdges & (-simdSize);
            const float stopScore = numEdges * layerMinScore - numEdges;

            if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint)) {
                continue;
            }

            int e = 0, j = 0;
            const float *tmplDx = ntd.gNXVals.data();
            const float *tmplDy = ntd.gNYVals.data();
            const cv::Point *pEdgePt = ntd.edgeLocs.data();
            float sumDot = 0.f;
            int32_t partVals[9][simdSize];

            for (; e < regularNumEdges; e += simdSize)
            {
                for (int i = 0; i < simdSize; ++i)
                {
                    fillSobelNeigborhood(pEdgePt, row, col, i, partVals);
                }

                sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);

                j += simdSize;
                float safeScore = stopScore + j * f;
                float greedyScore = layerMinScore * j;
                if (sumDot < std::min(safeScore, greedyScore)) { break; }

                tmplDx += simdSize;
                tmplDy += simdSize;
            }

            if (e < regularNumEdges) { continue; }

            if (e < numEdges)
            {
                std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
                for (int k=0; e < numEdges; ++e, ++k)
                {
                    fillSobelNeigborhood(pEdgePt, row, col, k, partVals);
                }
                sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
            }

            float score = sumDot / numEdges;
            if (score > maxScore)
            {
                maxScore = score;
                bestTmplIndex = tmplIndex;
            }
        }

        if (maxScore > layerMinScore)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = maxScore;
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
        }
    }
}

template<>
void ShapeCandidateScaner<true>::operator()(const tbb::blocked_range<int>& r) const
{
    constexpr int simdSize = 8;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle orb(1, layerMat.cols - 2, 1, layerMat.rows - 2);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * layer);
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const auto &tmplDatas = ltd.tmplDatas;
    const auto &upperTmplDatas = tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);

    for (int c = r.begin(); c != r.end(); ++c)
    {
        BaseTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        const cv::Point anchorPt{ col, row };
        float maxScore = -1.f;
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;

        for (const int tmplIndex : tmplIndices)
        {
            const ShapeTemplData &ntd = tmplDatas[tmplIndex];
            const int numEdges = static_cast<int>(ntd.edgeLocs.size());
            const int regularNumEdges = numEdges & (-simdSize);
            const float stopScore = numEdges * layerMinScore - numEdges;

            int e = 0, j = 0;
            const float *tmplDx = ntd.gNXVals.data();
            const float *tmplDy = ntd.gNYVals.data();
            const cv::Point *pEdgePt = ntd.edgeLocs.data();
            float sumDot = 0.f;
            int32_t partVals[9][simdSize];

            if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint))
            {
                for (; e < regularNumEdges; e += simdSize)
                {
                    for (int i = 0; i < simdSize; ++i)
                    {
                        const cv::Point ep{ pEdgePt->x + col, pEdgePt->y + row };
                        if (orb(ep))
                        {
                            fillSobelNeigborhood(pEdgePt, i, partVals);
                        }
                        else
                        {
                            fillSobelNeigborhood(pEdgePt, ep, i, partVals);
                        }
                    }

                    sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);

                    j += simdSize;
                    float safeScore = stopScore + j * f;
                    float greedyScore = layerMinScore * j;
                    if (sumDot < std::min(safeScore, greedyScore)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                }

                if (e < regularNumEdges) { continue; }

                if (e < numEdges)
                {
                    std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
                    for (int k = 0; e < numEdges; ++e, ++k)
                    {
                        const cv::Point ep{ pEdgePt->x + col, pEdgePt->y + row };
                        if (orb(ep))
                        {
                            fillSobelNeigborhood(pEdgePt, k, partVals);
                        }
                        else
                        {
                            fillSobelNeigborhood(pEdgePt, ep, k, partVals);
                        }
                    }
                    sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                }
            }
            else
            {
                for (; e < regularNumEdges; e += simdSize)
                {
                    for (int i = 0; i < simdSize; ++i)
                    {
                        fillSobelNeigborhood(pEdgePt, row, col, i, partVals);
                    }

                    sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);

                    j += simdSize;
                    float safeScore = stopScore + j * f;
                    float greedyScore = layerMinScore * j;
                    if (sumDot < std::min(safeScore, greedyScore)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                }

                if (e < regularNumEdges) { continue; }

                if (e < numEdges)
                {
                    std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
                    for (int k = 0; e < numEdges; ++e, ++k)
                    {
                        fillSobelNeigborhood(pEdgePt, row, col, k, partVals);
                    }
                    sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                }
            }

            float score = sumDot / numEdges;
            if (score > maxScore)
            {
                maxScore = score;
                bestTmplIndex = tmplIndex;
            }
        }

        if (maxScore > layerMinScore)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = maxScore;
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
        }
    }
}

template<>
float ShapeCandidateScaner<false>::getScore(const int row, const int col, const int ang) const
{
    constexpr int simdSize = 8;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle orb(1, layerMat.cols - 2, 1, layerMat.rows - 2);
    const auto &tmplDatas = ltd.tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);

    if (ang<0 || ang>=tmplDatas.size())
    {
        return -1.f;
    }

    const ShapeTemplData &ntd = tmplDatas[ang];
    const int numEdges = static_cast<int>(ntd.edgeLocs.size());
    const int regularNumEdges = numEdges & (-simdSize);

    const cv::Point anchorPt{ col, row };
    if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint))
    {
        return -1.f;
    }

    int32_t partVals[9][simdSize];
    const float *tmplDx = ntd.gNXVals.data();
    const float *tmplDy = ntd.gNYVals.data();

    int e = 0;
    float sumDot = 0.f;
    const cv::Point *pEdgePt = ntd.edgeLocs.data();
    for (; e < regularNumEdges; e += simdSize)
    {
        for (int i = 0; i < simdSize; ++i)
        {
            fillSobelNeigborhood(pEdgePt, row, col, i, partVals);
        }

        sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
        tmplDx += simdSize;
        tmplDy += simdSize;
    }

    if (e < numEdges)
    {
        std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
        for (int k=0; e < numEdges; ++e, ++k)
        {
            fillSobelNeigborhood(pEdgePt, row, col, k, partVals);
        }
        sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
    }

    float score = sumDot / numEdges;
    return score;
}

template<>
float ShapeCandidateScaner<true>::getScore(const int row, const int col, const int ang) const
{
    constexpr int simdSize = 8;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle orb(1, layerMat.cols - 2, 1, layerMat.rows - 2);
    const auto &tmplDatas = ltd.tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);

    if (ang < 0 || ang >= tmplDatas.size())
    {
        return -1.f;
    }

    const ShapeTemplData &ntd = tmplDatas[ang];
    const int numEdges = static_cast<int>(ntd.edgeLocs.size());
    const int regularNumEdges = numEdges & (-simdSize);

    int32_t partVals[9][simdSize];
    const float *tmplDx = ntd.gNXVals.data();
    const float *tmplDy = ntd.gNYVals.data();

    int e = 0;
    float sumDot = 0.f;
    const cv::Point *pEdgePt = ntd.edgeLocs.data();

    const cv::Point anchorPt{ col, row };
    if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint))
    {
        for (; e < regularNumEdges; e += simdSize)
        {
            for (int i = 0; i < simdSize; ++i)
            {
                const cv::Point ep{ pEdgePt->x + col, pEdgePt->y + row };
                if (orb(ep)) {
                    fillSobelNeigborhood(pEdgePt, i, partVals);
                } else {
                    fillSobelNeigborhood(pEdgePt, ep, i, partVals);
                }
            }

            sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
            tmplDx += simdSize;
            tmplDy += simdSize;
        }

        if (e < numEdges) {
            std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
            for (int k = 0; e < numEdges; ++e, ++k)
            {
                const cv::Point ep{ pEdgePt->x + col, pEdgePt->y + row };
                if (orb(ep)) {
                    fillSobelNeigborhood(pEdgePt, k, partVals);
                } else {
                    fillSobelNeigborhood(pEdgePt, ep, k, partVals);
                }
            }
            sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
        }
    }
    else
    {
        for (; e < regularNumEdges; e += simdSize)
        {
            for (int i = 0; i < simdSize; ++i)
            {
                fillSobelNeigborhood(pEdgePt, row, col, i, partVals);
            }

            sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
            tmplDx += simdSize;
            tmplDy += simdSize;
        }

        if (e < numEdges) {
            std::memset(partVals, 0, 9 * simdSize * sizeof(partVals[0][0]));
            for (int k = 0; e < numEdges; ++e, ++k)
            {
                fillSobelNeigborhood(pEdgePt, row, col, k, partVals);
            }
            sumDot += getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
        }
    }

    float score = sumDot / numEdges;
    return score;
}

template<>
void DeformCandidateScaner<false>::operator()(const tbb::blocked_range<int>& r) const
{
    constexpr int simdSize = 8;
    constexpr int maxDeform = 1;
    constexpr int safeDeform = maxDeform + 1;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle orb(1 + safeDeform, layerMat.cols - 2 - safeDeform, 1 + safeDeform, layerMat.rows - 2 - safeDeform);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * layer);
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const auto &tmplDatas = ltd.tmplDatas;
    const auto &upperTmplDatas = tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);
    ScalablePointList maxDeforms, curDeforms;

    for (int c = r.begin(); c != r.end(); ++c)
    {
        BaseTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        const cv::Point anchorPt{ col, row };
        float maxScore = -1.f;
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;

        for (const int tmplIndex : tmplIndices)
        {
            const ShapeTemplData &ntd = tmplDatas[tmplIndex];
            const int numEdges = static_cast<int>(ntd.edgeLocs.size());
            const int regularNumEdges = numEdges & (-simdSize);
            const float stopScore = numEdges * layerMinScore - numEdges;
            curDeforms.resize(ntd.clusters.size());

            if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint)) {
                continue;
            }

            int e = 0, j = 0;
            const float *tmplDx = ntd.gNXVals.data();
            const float *tmplDy = ntd.gNYVals.data();
            const cv::Point *pEdgePt = ntd.edgeLocs.data();
            float sumDot = 0.f;
            int32_t partVals[9][simdSize];
            cv::Point clusterEdgePts[8];

            for (int cluster = 0; e < regularNumEdges; e += simdSize, ++cluster)
            {
                curDeforms[cluster].x = 0;
                curDeforms[cluster].y = 0;
                float maxClusterScore = std::numeric_limits<float>::lowest();
                if (ntd.clusters[cluster].label)
                {
                    for (int v = -maxDeform; v < maxDeform + 1; ++v)
                    {
                        const cv::Point deform = ntd.clusters[cluster].direction * v;
                        deformClusterEdgePoints(pEdgePt, deform, clusterEdgePts);
                        for (int i = 0; i < simdSize; ++i)
                        {
                            fillSobelNeigborhood(clusterEdgePts, row, col, i, partVals);
                        }
                        const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                        if (clusterScore > maxClusterScore )
                        {
                            maxClusterScore = clusterScore;
                            curDeforms[cluster].x = v;
                        }
                    }
                }
                else
                {
                    for (int y = -maxDeform; y < maxDeform + 1; ++y)
                    {
                        for (int x = -maxDeform; x < maxDeform + 1; ++x)
                        {
                            const cv::Point *pClusterEdgePt = pEdgePt;
                            for (int i = 0; i < simdSize; ++i)
                            {
                                fillSobelNeigborhood(pClusterEdgePt, row+y, col+x, i, partVals);
                            }
                            const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                            if (clusterScore > maxClusterScore)
                            {
                                maxClusterScore = clusterScore;
                                curDeforms[cluster].x = x;
                                curDeforms[cluster].y = y;
                            }
                        }
                    }
                }

                sumDot += maxClusterScore;

                j += simdSize;
                float safeScore = stopScore + j * f;
                float greedyScore = layerMinScore * j;
                if (sumDot < std::min(safeScore, greedyScore)) { break; }

                tmplDx += simdSize;
                tmplDy += simdSize;
                pEdgePt += simdSize;
            }

            if (e < regularNumEdges) { continue; }

            float score = sumDot / numEdges;
            if (score > maxScore)
            {
                maxScore = score;
                bestTmplIndex = tmplIndex;
                maxDeforms.swap(curDeforms);
            }
        }

        if (maxScore > layerMinScore)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = maxScore;
            candidate.deforms.swap(maxDeforms);
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
            candidate.deforms.resize(0);
        }
    }
}

template<>
void DeformCandidateScaner<true>::operator()(const tbb::blocked_range<int>& r) const
{
    constexpr int simdSize = 8;
    constexpr int maxDeform = 1;
    constexpr int safeDeform = maxDeform + 1;
    const LayerShapeData &ltd = tmpl->pyramid_tmpl_datas_[layer];
    const cv::Mat &layerMat = tmpl->pyrs_[layer];
    OutsideRectangle oir(1, layerMat.cols - 2, 1, layerMat.rows - 2);
    OutsideRectangle orb(1 + safeDeform, layerMat.cols - 2 - safeDeform, 1 + safeDeform, layerMat.rows - 2 - safeDeform);
    const float layerMinScore = std::max(0.4f, minScore - 0.1f * layer);
    const float f = (1.f - greediness * layerMinScore) / (1.f - greediness);
    const auto &tmplDatas = ltd.tmplDatas;
    const auto &upperTmplDatas = tmpl->pyramid_tmpl_datas_[layer + 1].tmplDatas;
    vcl::Vec8i vecMinContrast(minContrast*minContrast * 64);

    for (int c = r.begin(); c != r.end(); ++c)
    {
        BaseTemplate::Candidate &candidate = tmpl->final_candidates_[c];
        const int row = candidate.row;
        const int col = candidate.col;
        const cv::Point anchorPt{ col, row };
        float maxScore = -1.f;
        int bestTmplIndex = 0;
        const std::vector<int> &tmplIndices = upperTmplDatas[candidate.mindex].mindices;

        for (const int tmplIndex : tmplIndices)
        {
            const ShapeTemplData &ntd = tmplDatas[tmplIndex];
            const int numEdges = static_cast<int>(ntd.edgeLocs.size());
            const int regularNumEdges = numEdges & (-simdSize);
            const float stopScore = numEdges * layerMinScore - numEdges;

            int e = 0, j = 0;
            const float *tmplDx = ntd.gNXVals.data();
            const float *tmplDy = ntd.gNYVals.data();
            const cv::Point *pEdgePt = ntd.edgeLocs.data();
            float sumDot = 0.f;
            int32_t partVals[9][simdSize];
            cv::Point clusterEdgePts[8];

            if (orb(anchorPt + ntd.minPoint) || orb(anchorPt + ntd.maxPoint))
            {
                for (int cluster = 0; e < regularNumEdges; e += simdSize, ++cluster)
                {
                    float maxClusterScore = std::numeric_limits<float>::lowest();
                    if (ntd.clusters[cluster].label)
                    {
                        for (int v = -maxDeform; v < maxDeform + 1; ++v)
                        {
                            const cv::Point deform = ntd.clusters[cluster].direction * v;
                            deformClusterEdgePoints(pEdgePt, deform, clusterEdgePts);
                            const cv::Point *pClusterEdgePt = clusterEdgePts;
                            for (int i = 0; i < simdSize; ++i)
                            {
                                const cv::Point ep{ pClusterEdgePt->x + col, pClusterEdgePt->y + row };
                                if (oir(ep))
                                {
                                    fillSobelNeigborhood(pClusterEdgePt, i, partVals);
                                }
                                else
                                {
                                    fillSobelNeigborhood(pClusterEdgePt, ep, i, partVals);
                                }
                            }
                            const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                            if (clusterScore > maxClusterScore)
                            {
                                maxClusterScore = clusterScore;
                            }
                        }
                    }
                    else
                    {
                        for (int y = -maxDeform; y < maxDeform + 1; ++y)
                        {
                            for (int x = -maxDeform; x < maxDeform + 1; ++x)
                            {
                                const cv::Point *pClusterEdgePt = pEdgePt;
                                for (int i = 0; i < simdSize; ++i)
                                {
                                    const cv::Point ep{ pClusterEdgePt->x + col + x, pClusterEdgePt->y + row + y };
                                    if (oir(ep))
                                    {
                                        fillSobelNeigborhood(pClusterEdgePt, i, partVals);
                                    }
                                    else
                                    {
                                        fillSobelNeigborhood(pClusterEdgePt, ep, i, partVals);
                                    }
                                }
                                const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                                if (clusterScore > maxClusterScore)
                                {
                                    maxClusterScore = clusterScore;
                                }
                            }
                        }
                    }

                    sumDot += maxClusterScore;

                    j += simdSize;
                    float safeScore = stopScore + j * f;
                    float greedyScore = layerMinScore * j;
                    if (sumDot < std::min(safeScore, greedyScore)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                    pEdgePt += simdSize;
                }
            }
            else
            {
                for (int cluster = 0; e < regularNumEdges; e += simdSize, ++cluster)
                {
                    float maxClusterScore = std::numeric_limits<float>::lowest();
                    if (ntd.clusters[cluster].label)
                    {
                        for (int v = -maxDeform; v < maxDeform + 1; ++v)
                        {
                            const cv::Point deform = ntd.clusters[cluster].direction * v;
                            deformClusterEdgePoints(pEdgePt, deform, clusterEdgePts);
                            for (int i = 0; i < simdSize; ++i)
                            {
                                fillSobelNeigborhood(clusterEdgePts, row, col, i, partVals);
                            }
                            const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                            if (clusterScore > maxClusterScore)
                            {
                                maxClusterScore = clusterScore;
                            }
                        }
                    }
                    else
                    {
                        for (int y = -maxDeform; y < maxDeform + 1; ++y)
                        {
                            for (int x = -maxDeform; x < maxDeform + 1; ++x)
                            {
                                const cv::Point *pClusterEdgePt = pEdgePt;
                                for (int i = 0; i < simdSize; ++i)
                                {
                                    fillSobelNeigborhood(pClusterEdgePt, row + y, col + x, i, partVals);
                                }
                                const float clusterScore = getPartScore(partVals, vecMinContrast, tmplDx, tmplDy);
                                if (clusterScore > maxClusterScore)
                                {
                                    maxClusterScore = clusterScore;
                                }
                            }
                        }
                    }

                    sumDot += maxClusterScore;

                    j += simdSize;
                    float safeScore = stopScore + j * f;
                    float greedyScore = layerMinScore * j;
                    if (sumDot < std::min(safeScore, greedyScore)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                    pEdgePt += simdSize;
                }
            }

            if (e < regularNumEdges) { continue; }

            float score = sumDot / numEdges;
            if (score > maxScore)
            {
                maxScore = score;
                bestTmplIndex = tmplIndex;
            }
        }

        if (maxScore > layerMinScore)
        {
            candidate.mindex = bestTmplIndex;
            candidate.score = maxScore;
        }
        else
        {
            candidate.mindex = -1;
            candidate.score = 0.f;
            candidate.deforms.resize(0);
        }
    }
}

ShapeTemplate::ShapeTemplate()
{
}

ShapeTemplate::~ShapeTemplate()
{
}

SpamResult ShapeTemplate::matchShapeTemplate(const cv::Mat &img,
    const ShapeTmplMatchOption &smo,
    cv::Point2f &pos,
    float &angle,
    float &score)
{
    initMatchResult(pos, angle, score);
    clearCacheMatchData();

    if (pyramid_tmpl_datas_.empty() ||
        pyramid_level_ != static_cast<int>(pyramid_tmpl_datas_.size()))
    {
        return SpamResult::kSR_TM_CORRUPTED_TEMPL_DATA;
    }

    cv::buildPyramid(img, pyrs_, pyramid_level_ - 1, cv::BORDER_REFLECT);
    const cv::Mat &topLayer = pyrs_.back();
    const int nTopRows = topLayer.rows;
    const int nTopCols = topLayer.cols;

    SpamGradient::SobelNormalize(topLayer, top_layer_dx_, top_layer_dy_, cv::Rect(0, 0, nTopCols, nTopRows), smo.minContrast);

    dx_row_ptrs_.clear();
    dy_row_ptrs_.clear();
    dx_row_ptrs_.resize(nTopRows);
    dy_row_ptrs_.resize(nTopRows);
    for (int row = 0; row < nTopRows; ++row)
    {
        dx_row_ptrs_[row] = top_layer_dx_.ptr<float>(row);
        dy_row_ptrs_[row] = top_layer_dy_.ptr<float>(row);
    }

    int numSearchRuns = 0;
    const SpamRun *searchRuns = nullptr;
    std::tie(numSearchRuns, searchRuns) = getSearchRegion(nTopRows, nTopCols);

    if (smo.touchBorder)
    {
        ShapeTopLayerScaner<true> bfNCCScaner(this, searchRuns, smo.minScore, smo.greediness);
        bfNCCScaner(tbb::blocked_range<int>(0, numSearchRuns));
        candidates_.swap(bfNCCScaner.candidates);
    }
    else
    {
        ShapeTopLayerScaner<false> bfNCCScaner(this, searchRuns, smo.minScore, smo.greediness);
        bfNCCScaner(tbb::blocked_range<int>(0, numSearchRuns));
        candidates_.swap(bfNCCScaner.candidates);
    }

    supressNoneMaximum();
    startMoveCandidatesToLowerLayer();

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

        if (layer < 2)
        {
            if (smo.touchBorder)
            {
                DeformCandidateScaner<true> dcs(this, smo.minScore, smo.greediness, layer, smo.minContrast);
                tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), dcs);
            }
            else
            {
                DeformCandidateScaner<false> dcs(this, smo.minScore, smo.greediness, layer, smo.minContrast);
                tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), dcs);
            }
        }
        else
        {
            if (smo.touchBorder)
            {
                ShapeCandidateScaner<true> scs(this, smo.minScore, smo.greediness, layer, smo.minContrast);
                tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), scs);
            }
            else
            {
                ShapeCandidateScaner<false> scs(this, smo.minScore, smo.greediness, layer, smo.minContrast);
                tbb::parallel_for(tbb::blocked_range<int>(0, static_cast<int>(final_candidates_.size())), scs);
            }
        }

        moveCandidatesToLowerLayer(layer);
    }

    BaseTemplate::Candidate bestCandidate(0, 0, std::numeric_limits<float>::lowest());
    for (const BaseTemplate::Candidate &candidate : final_candidates_)
    {
        if (candidate.score > bestCandidate.score)
        {
            bestCandidate = candidate;
        }
    }

    if (bestCandidate.score > smo.minScore)
    {
        cv::Point2f subPos{ 0.f, 0.f };
        float       subAng{ 0.f };
        const auto &tmplDatas = pyramid_tmpl_datas_[0].tmplDatas;
        pos.x = static_cast<float>(bestCandidate.col);
        pos.y = static_cast<float>(bestCandidate.row);
        angle = tmplDatas[bestCandidate.mindex].angle;
        score = bestCandidate.score;

        if (score < 0.99f)
        {
            float subScore = estimateSubPixelPose(bestCandidate, smo.minScore, smo.minContrast, smo.greediness, subPos, subAng);
            if (subScore > score && std::abs(subPos.x) < 0.618f && std::abs(subPos.y) < 0.618f && std::abs(subAng) < 0.618f)
            {
                pos.x = static_cast<float>(bestCandidate.col) + subPos.x;
                pos.y = static_cast<float>(bestCandidate.row) + subPos.y;
                angle = tmplDatas[bestCandidate.mindex].angle + subAng * pyramid_tmpl_datas_[0].angleStep;
                score = std::min(subScore, 1.f);
            }
        }

        return SpamResult::kSR_SUCCESS;
    }
    else
    {
        return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
    }
}

SpamResult ShapeTemplate::CreateTemplate(const ShapeTmplCreateData &createData)
{
    destroyData();
    SpamResult sr = verifyCreateData(createData.baseData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    sr = createShapeTemplate(createData);
    if (SpamResult::kSR_SUCCESS != sr)
    {
        destroyData();
        return sr;
    }

    linkTemplatesBetweenLayers();

    pyramid_level_ = createData.baseData.pyramidLevel;
    processToplayerSearchROI(createData.baseData.roi, createData.baseData.pyramidLevel);

    return SpamResult::kSR_SUCCESS;
}

cv::Mat ShapeTemplate::GetTopScoreMat() const
{
    cv::Mat scoreMat(pyrs_.back().rows, pyrs_.back().cols, CV_8UC1, cv::Scalar());
    for (const auto &candidate : top_candidates_)
    {
        scoreMat.at<uint8_t>(cv::Point(candidate.col, candidate.row)) = cv::saturate_cast<uint8_t>(candidate.score * 255);
    }
    return scoreMat;
}

void ShapeTemplate::DumpTemplate(std::ostream &oss) const
{
    for (std::size_t l=0; l<pyramid_tmpl_datas_.size(); ++l)
    {
        const LayerShapeData &lsd = pyramid_tmpl_datas_[l];
        for (std::size_t t = 0; t < lsd.tmplDatas.size(); ++t)
        {
            const ShapeTemplData &shtd = lsd.tmplDatas[t];
            for (std::size_t e = 0; e < shtd.edgeLocs.size(); ++e)
            {
                oss << "[" << std::setw(3) << l;
                oss << "," << std::setw(3) << t;
                oss << "," << std::setw(5) << e;
                oss << "," << std::setw(11) << std::fixed << std::setprecision(6) << shtd.angle;
                oss << "," << std::setw(5) << shtd.edgeLocs[e].x;
                oss << "," << std::setw(5) << shtd.edgeLocs[e].y;
                oss << "," << std::setw(10) << std::fixed << std::setprecision(6) << shtd.gNXVals[e];
                oss << "," << std::setw(10) << std::fixed << std::setprecision(6) << shtd.gNYVals[e];
                oss << "]" << std::endl;
            }
        }
    }
}

void ShapeTemplate::DrawTemplate(cv::Mat &img, const cv::Point2f &pos, const float angle) const
{
    if (!pyramid_tmpl_datas_.empty())
    {
        const LayerShapeData &lsd = pyramid_tmpl_datas_.front();
        if (!lsd.tmplDatas.empty())
        {
            const ShapeTemplData &shtd = lsd.tmplDatas.front();
            cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f(0.f, 0.f), angle- shtd.angle, 1.0);
            rotMat.at<double>(0, 2) += pos.x;
            rotMat.at<double>(1, 2) += pos.y;

            std::vector<cv::Point2d> tmplEdgePoints;
            tmplEdgePoints.reserve(shtd.edgeLocs.size());
            for (const cv::Point &pt : shtd.edgeLocs)
            {
                tmplEdgePoints.emplace_back(pt.x, pt.y);
            }

            std::vector<cv::Point2d> transEdgePoints;
            cv::transform(tmplEdgePoints, transEdgePoints, rotMat);

            OutsideImageBox oib(img.cols, img.rows);
            for (const cv::Point2d &ept : transEdgePoints)
            {
                const cv::Point pt = ept;
                if (!oib(pt))
                {
                    img.at<cv::Vec4b>(pt) = cv::Vec4b(0x00, 0x00, 0xFF, 0xFF);
                }
            }
        }
    }
}

void ShapeTemplate::destroyData()
{
    destroyBaseData();
    pyramid_tmpl_datas_.clear();
}

SpamResult ShapeTemplate::createShapeTemplate(const ShapeTmplCreateData &createData)
{
    if (createData.baseData.tmplRgn.empty())
    {
        return SpamResult::kSR_TM_EMPTY_TEMPL_REGION;
    }

    std::vector<Geom::PathVector> tmplRgns;
    tmplRgns.push_back(createData.baseData.tmplRgn);

    double s = 0.5;
    for (int l = 1; l < createData.baseData.pyramidLevel; ++l)
    {
        tmplRgns.push_back(createData.baseData.tmplRgn*Geom::Scale(s, s));
        s *= 0.5;
    }

    cv::Mat tmplImg;
    cv::medianBlur(createData.baseData.srcImg, tmplImg, 5);
    cv::buildPyramid(tmplImg, pyrs_, createData.baseData.pyramidLevel - 1, cv::BORDER_REFLECT);
    SpamRgn maskRgn(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(0, 0), 5))));
    PointSet maskPoints(maskRgn);

    cv::Mat transPyrImg;
    cv::Mat transRotPyrImg;
    cv::Mat dx, dy, ndx, ndy, edges;
    cv::Mat transMat = (cv::Mat_<double>(2, 3) << 1, 0, 0, 0, 1, 0);
    std::vector<uint8_t> maskBuf;

    for (int l = 0; l < createData.baseData.pyramidLevel; ++l)
    {
        SpamRgn rgn(tmplRgns[l], maskBuf);
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
        LayerShapeData &ltd = pyramid_tmpl_datas_.back();

        const cv::Mat &pyrImg = pyrs_[l];
        const cv::Point pyrImgCenter{ pyrImg.cols / 2, pyrImg.rows / 2 };
        auto &tmplDatas = ltd.tmplDatas;

        transMat.at<double>(0, 2) = pyrImgCenter.x - anchorPoint.x;
        transMat.at<double>(1, 2) = pyrImgCenter.y - anchorPoint.y;
        cv::warpAffine(pyrImg, transPyrImg, transMat, cv::Size(pyrImg.cols, pyrImg.rows));
        std::map<int, uint8_t> minMaxDiffs;

        for (double ang = 0; ang < createData.baseData.angleExtent; ang += angleStep)
        {
            const double deg = createData.baseData.angleStart + ang;
            SpamRgn originRgn(tmplRgns[l] * Geom::Translate(-anchorPoint.x, -anchorPoint.y)* Geom::Rotate::from_degrees(-deg), maskBuf);

            PointSet pointSetOrigin(originRgn);
            PointSet pointSetTmpl(originRgn, pyrImgCenter);

            cv::Mat rotMat = cv::getRotationMatrix2D(pyrImgCenter, deg, 1.0);
            cv::warpAffine(transPyrImg, transRotPyrImg, rotMat, cv::Size(transPyrImg.cols, transPyrImg.rows));

            if (!pointSetTmpl.IsInsideImage(cv::Size(transRotPyrImg.cols, transRotPyrImg.rows)))
            {
                return SpamResult::kSR_TM_TEMPL_REGION_OUT_OF_RANGE;
            }

            cv::Point minPt, maxPt;
            std::tie(minPt, maxPt) = pointSetTmpl.MinMax();
            cv::Rect rcROI{ minPt, maxPt + cv::Point(1, 1) };

            SpamGradient::Sobel(transRotPyrImg, dx, dy, rcROI);
            SpamGradient::SobelNormalize(transRotPyrImg, ndx, ndy, rcROI, 0);
            cv::Canny(dx, dy, edges, createData.lowContrast, createData.highContrast, true);

            tmplDatas.emplace_back(static_cast<float>(deg), 1.f);
            ShapeTemplData &ptd = tmplDatas.back();

            PointSet &edgeLocs = ptd.edgeLocs;
            const int numPoints = static_cast<int>(pointSetTmpl.size());
            for (int n = 0; n < numPoints; ++n)
            {
                uint8_t minMaxDiff = 0;
                const cv::Point &tmplPt = pointSetTmpl[n];
                if (edges.at<uint8_t>(tmplPt))
                {
                    edgeLocs.push_back(pointSetOrigin[n]);
                    ptd.gNXVals.push_back(ndx.at<float>(tmplPt));
                    ptd.gNYVals.push_back(ndy.at<float>(tmplPt));
                }
            }

            if (edgeLocs.size() < 9)
            {
                return SpamResult::kSR_TM_TEMPL_INSIGNIFICANT;
            }

            const auto minMaxPoints = edgeLocs.MinMax();
            ptd.minPoint = minMaxPoints.first;
            ptd.maxPoint = minMaxPoints.second;

            if (l == createData.baseData.pyramidLevel-1)
            {
                constexpr int simdSize = 8;
                GradientSequence gRegNXVals(((static_cast<int>(ptd.gNXVals.size()) + simdSize - 1) & (-simdSize)), 0.f);
                float *pRegXVals = gRegNXVals.data();
                for (float dx : ptd.gNXVals)
                {
                    *pRegXVals = dx; pRegXVals += 1;
                }
                ptd.gNXVals.swap(gRegNXVals);

                GradientSequence gRegNYVals(((static_cast<int>(ptd.gNYVals.size()) + simdSize - 1) & (-simdSize)), 0.f);
                float *pRegYVals = gRegNYVals.data();
                for (float dy : ptd.gNYVals)
                {
                    *pRegYVals = dy; pRegYVals += 1;
                }
                ptd.gNYVals.swap(gRegNYVals);
            }
            else
            {
                groupEdgePoints(ptd);
            }

            AngleRange<double> angleRange{ deg , deg + angleStep };
            if (angleRange.between(0))
            {
                ang = -angleStep - createData.baseData.angleStart;
            }
        }
    }

    return SpamResult::kSR_OK;
}

void ShapeTemplate::linkTemplatesBetweenLayers()
{
    const int topLayerIndex = static_cast<int>(pyramid_tmpl_datas_.size() - 1);
    for (int layer = topLayerIndex; layer > 0; --layer)
    {
        LayerShapeData &ltd = pyramid_tmpl_datas_[layer];
        LayerShapeData &belowLtd = pyramid_tmpl_datas_[layer - 1];
        auto &tmplDatas = ltd.tmplDatas;
        const auto &belowTmplDatas = belowLtd.tmplDatas;
        for (ShapeTemplData &ptd : tmplDatas)
        {
            AngleRange<float> angleRange(ptd.angle - ltd.angleStep, ptd.angle + ltd.angleStep);
            for (int t = 0; t < belowTmplDatas.size(); ++t)
            {
                if (angleRange.contains(belowTmplDatas[t].angle))
                {
                    ptd.mindices.push_back(t);
                }
            }
        }
    }
}

float ShapeTemplate::estimateSubPixelPose(const Candidate &bestCandidate,
    const float minScore,
    const int minContrast,
    const float greediness,
    cv::Point2f &pos,
    float &angle)
{
    int i = 0;
    cv::Mat scores(27, 1, CV_64FC1);
    ShapeCandidateScaner<true> scs(this, minScore, greediness, 0, minContrast);

    for (int r = -1; r < 2; ++r)
    {
        for (int c = -1; c < 2; ++c)
        {
            for (int a = -1; a < 2; ++a)
            {
                const int row = r + bestCandidate.row;
                const int col = c + bestCandidate.col;
                const int ang = a + bestCandidate.mindex;
                const float s = scs.getScore(row, col, ang);
                if (s<0.f)
                {
                    return s;
                }
                scores.ptr<double>(i++)[0] = s;
            }
        }
    }

    return maxScoreInterpolate(scores, pos, angle);
}

void ShapeTemplate::groupEdgePoints(ShapeTemplData &shptd) const
{
    std::vector<bool> closed;
    std::vector<std::vector<int>> curves;
    BasicImgProc::TrackCurves(shptd.edgeLocs, shptd.minPoint, shptd.maxPoint, curves, closed);
    BasicImgProc::SplitCurvesToSegments(closed, curves);

    GradientSequence gNXVals;
    GradientSequence gNYVals;
    PointSet         edgeLocs;
    ClusterSequence  clusters;

    for (const std::vector<int> &curve : curves)
    {
        clusters.emplace_back();
        clusters.back().direction.x = 0.f;
        clusters.back().direction.y = 0.f;

        for (const int eIdx : curve)
        {
            gNXVals.push_back(shptd.gNXVals[eIdx]);
            gNYVals.push_back(shptd.gNYVals[eIdx]);
            edgeLocs.push_back(shptd.edgeLocs[eIdx]);

            clusters.back().direction.x += shptd.gNXVals[eIdx];
            clusters.back().direction.y += shptd.gNYVals[eIdx];
            clusters.back().center += cv::Point2f(shptd.edgeLocs[eIdx]);
        }

        clusters.back().center /= 8.f;
        clusters.back().direction /= 8.f;
        const auto l2 = cv::norm(clusters.back().direction);
        if (l2 > 0.55f)
        {
            clusters.back().direction /= l2;
            clusters.back().label = 1;
        }
        else
        {
            clusters.back().label = 0;
        }
    }

    std::vector<int> idxs(clusters.size());
    std::iota(idxs.begin(), idxs.end(), 0);
    std::sort(idxs.begin(), idxs.end(), [&clusters](const int l, const int r) { return (clusters[l].center.y < clusters[r].center.y) || (clusters[l].center.y == clusters[r].center.y && clusters[l].center.x < clusters[r].center.x); });

    shptd.gNXVals.resize(0);
    shptd.gNYVals.resize(0);
    shptd.edgeLocs.resize(0);
    shptd.gNXVals.reserve(clusters.size() * 8);
    shptd.gNYVals.reserve(clusters.size() * 8);
    shptd.edgeLocs.reserve(clusters.size() * 8);

    for (const int idx : idxs)
    {
        shptd.gNXVals.insert(shptd.gNXVals.end(), gNXVals.cbegin() + idx * 8, gNXVals.cbegin() + idx * 8 + 8);
        shptd.gNYVals.insert(shptd.gNYVals.end(), gNYVals.cbegin() + idx * 8, gNYVals.cbegin() + idx * 8 + 8);
        shptd.edgeLocs.insert(shptd.edgeLocs.end(), edgeLocs.cbegin() + idx * 8, edgeLocs.cbegin() + idx * 8 + 8);
        shptd.clusters.push_back(clusters[idx]);
    }
}