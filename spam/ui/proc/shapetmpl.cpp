#include "shapetmpl.h"
#include "gradient.h"
#include "basic.h"
#include <limits>
#include <stack>
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
    ShapeTopLayerScaner(const ShapeTemplate *const shapeTmpl, const SpamRun *const r, const float s) : tmpl(shapeTmpl), roi(r), score(s) {}
    ShapeTopLayerScaner(ShapeTopLayerScaner& s, tbb::split) : tmpl(s.tmpl), roi(s.roi), score(s.score) { }

    void operator()(const tbb::blocked_range<int>& br);
    void join(ShapeTopLayerScaner& rhs) { candidates.insert(candidates.end(), rhs.candidates.cbegin(), rhs.candidates.cend()); }
    static float dotProduct(const float *xVec0, const float *yVec0, const float *xVec1, const float *yVec1);

    const float score;
    const ShapeTemplate *const tmpl;
    const SpamRun *const roi;
    BaseTemplate::CandidateList candidates;
};

template<bool TouchBorder>
inline float ShapeTopLayerScaner<TouchBorder>::dotProduct(const float *xVec0, const float *yVec0,
    const float *xVec1, const float *yVec1)
{
    vcl::Vec8f xvec0, yvec0, xvec1, yvec1;
    xvec0.load(xVec0);
    yvec0.load(yVec0);
    xvec1.load(xVec1);
    yvec1.load(yVec1);
    vcl::Vec8f dot = xvec0 * xvec1 + yvec0 * yvec1;
    return vcl::horizontal_add(dot);
}

template<>
void ShapeTopLayerScaner<false>::operator()(const tbb::blocked_range<int>& br)
{
    constexpr int simdSize = 8;
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerShapeData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const float layerMinScore = std::max(0.5f, score - 0.1f * (tmpl->pyramid_level_ - 1));
    const int runStart = br.begin();
    const int runEnd = br.end();
    const std::vector<ShapeTemplData> &tmplDatas = layerTempls.tmplDatas;
    const int numLayerTempls = static_cast<int>(tmplDatas.size());

    std::array<float, simdSize> partDXVals, partDYVals;
    std::map<int, BaseTemplate::Candidate, std::less<int>, tbb::scalable_allocator<std::pair<const int, BaseTemplate::Candidate>>> aCandidates;

    for (int t = 0; t < numLayerTempls; ++t)
    {
        const ShapeTemplData &ntd = tmplDatas[t];
        const int numEdges = static_cast<int>(ntd.edgeLocs.size());
        const int regularNumEdges = numEdges & (-simdSize);
        const float tmplScore = numEdges * layerMinScore - numEdges;

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
                cv::Point anchorPt{ col, row };

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
                    if (sumDot < (tmplScore + j)) { break; }

                    tmplDx += simdSize;
                    tmplDy += simdSize;
                }

                if (e < regularNumEdges) { continue; }

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

                float score = sumDot / numEdges;
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
        }
    }

    candidates.reserve(aCandidates.size());
    for (const auto &cItem : aCandidates)
    {
        candidates.push_back(cItem.second);
    }
}

template<>
void ShapeTopLayerScaner<true>::operator()(const tbb::blocked_range<int>& br)
{
    constexpr int simdSize = 8;
    const cv::Mat &layerMat = tmpl->pyrs_.back();
    const int nLayerCols = layerMat.cols;
    const LayerShapeData &layerTempls = tmpl->pyramid_tmpl_datas_.back();
    OutsideImageBox oib(layerMat.cols, layerMat.rows);
    const float layerMinScore = std::max(0.5f, score - 0.1f * (tmpl->pyramid_level_ - 1));
    const int runStart = br.begin();
    const int runEnd = br.end();
    const std::vector<ShapeTemplData> &tmplDatas = layerTempls.tmplDatas;
    const int numLayerTempls = static_cast<int>(tmplDatas.size());

    std::array<float, simdSize> partDXVals, partDYVals;
    std::map<int, BaseTemplate::Candidate, std::less<int>, tbb::scalable_allocator<std::pair<const int, BaseTemplate::Candidate>>> aCandidates;

    for (int t = 0; t < numLayerTempls; ++t)
    {
        const ShapeTemplData &ntd = tmplDatas[t];
        const int numEdges = static_cast<int>(ntd.edgeLocs.size());
        const int regularNumEdges = numEdges & (-simdSize);
        const float tmplScore = numEdges * layerMinScore - numEdges;

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
                cv::Point anchorPt{ col, row };

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
                        if (sumDot < (tmplScore + j)) { break; }

                        tmplDx += simdSize;
                        tmplDy += simdSize;
                    }

                    if (e < regularNumEdges) { continue; }

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
                        if (sumDot < (tmplScore + j)) { break; }

                        tmplDx += simdSize;
                        tmplDy += simdSize;
                    }

                    if (e < regularNumEdges) { continue; }

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

                float score = sumDot / numEdges;
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
        }
    }

    candidates.reserve(aCandidates.size());
    for (const auto &cItem : aCandidates)
    {
        candidates.push_back(cItem.second);
    }
}

ShapeTemplate::ShapeTemplate()
{
}

ShapeTemplate::~ShapeTemplate()
{ 
}

SpamResult ShapeTemplate::matchShapeTemplate(const cv::Mat &img, const float minScore, const int minContrast, cv::Point2f &pos, float &angle, float &score)
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

    SpamGradient::SobelNormalize(topLayer, top_layer_dx_, top_layer_dy_, cv::Rect(0, 0, nTopCols, nTopRows), minContrast);

    dx_row_ptrs_.clear();
    dy_row_ptrs_.clear();
    dx_row_ptrs_.resize(nTopRows);
    dy_row_ptrs_.resize(nTopRows);
    for (int row = 0; row < nTopRows; ++row)
    {
        dx_row_ptrs_[row] = top_layer_dx_.ptr<float>(row);
        dy_row_ptrs_[row] = top_layer_dy_.ptr<float>(row);
    }

    if (top_layer_search_roi_.GetData().empty())
    {
        top_layer_full_domain_.SetRegion(cv::Rect(0, 0, nTopCols, nTopRows));
        const int numRuns = static_cast<int>(top_layer_full_domain_.GetData().size());
        ShapeTopLayerScaner<true> bfNCCScaner(this, top_layer_full_domain_.GetData().data(), minScore);
        //tbb::parallel_reduce(tbb::blocked_range<int>(0, numRuns), bfNCCScaner);
        bfNCCScaner(tbb::blocked_range<int>(0, numRuns));
        candidates_.swap(bfNCCScaner.candidates);
    }
    else
    {
        const int numRuns = static_cast<int>(top_layer_search_roi_.GetData().size());
        ShapeTopLayerScaner<true> bfNCCScaner(this, top_layer_search_roi_.GetData().data(), minScore);
        //tbb::parallel_reduce(tbb::blocked_range<int>(0, numRuns), bfNCCScaner);
        bfNCCScaner(tbb::blocked_range<int>(0, numRuns));
        candidates_.swap(bfNCCScaner.candidates);
    }

    supressNoneMaximum();

    return SpamResult::kSR_TM_INSTANCE_NOT_FOUND;
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
    cv::buildPyramid(createData.baseData.srcImg, pyrs_, createData.baseData.pyramidLevel - 1, cv::BORDER_REFLECT);
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

            const auto minMaxPoints = edgeLocs.MinMax();
            ptd.minPoint = minMaxPoints.first;
            ptd.maxPoint = minMaxPoints.second;

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