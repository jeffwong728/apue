#include "pixeltmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
#include <vectorclass/vectorclass.h>
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

PixelTemplate::PixelTemplate()
    : pyramid_level_(4)
{
}

PixelTemplate::~PixelTemplate()
{ 
}

SpamResult PixelTemplate::matchTemplate(const cv::Mat &img, const int sad, cv::Point2f &pos, float &angle)
{
    if (pyramid_tmpl_datas_.empty() || pyramid_level_ != static_cast<int>(pyramid_tmpl_datas_.size()))
    {
        return SpamResult::kSR_TM_CORRUPTED_TEMPL_DATA;
    }

    cv::buildPyramid(img, pyrs_, pyramid_level_-1, cv::BORDER_REFLECT);
    const cv::Mat &topLayer = pyrs_.back();

    top_layer_score_.create(topLayer.rows, topLayer.cols, CV_8UC1);
    top_layer_angle_.create(topLayer.rows, topLayer.cols, CV_32F);

    top_layer_score_ = cv::Scalar(0);
    top_layer_angle_ = cv::Scalar(0);

    const int topSad = sad + 5 * (pyramid_level_ - 1);
    auto outsideImg = [&topLayer](const cv::Point &point) { if (point.x < 0 || point.x >= topLayer.cols || point.y < 0 || point.y >= topLayer.rows) { return true; } else { return false; }};
    const LayerTmplData &topLayerTmplData = pyramid_tmpl_datas_.back();

    constexpr int32_t simdSize = 16;
    std::array<int16_t, simdSize> tempData;
    const int nRows = topLayer.rows;
    const int nCols = topLayer.cols;

    std::vector<const uint8_t *> pRows(nRows);
    for (int row = 0; row < nRows; ++row)
    {
        pRows[row] = topLayer.ptr<uint8_t>(row);
    }

    for (int row = 0; row < nRows; ++row)
    {
        for (int col = 0; col < nCols; ++col)
        {
            cv::Point originPt{col, row};
            int32_t minSAD = std::numeric_limits<int32_t>::max();
            float minAng = topLayerTmplData.tmplDatas.front().angle;
            for (int t = 0; t < static_cast<int>(topLayerTmplData.tmplDatas.size()); ++t)
            {
                const PixelTmplData &ptd = topLayerTmplData.tmplDatas[t];

                if (outsideImg(originPt + ptd.minPoint) || outsideImg(originPt + ptd.maxPoint))
                {
                    continue;
                }

                const int32_t numPoints = static_cast<int32_t>(ptd.pixlLocs.size());
                const int32_t regularSize = numPoints & (-simdSize);
                const auto &pixlVals = boost::get<std::vector<int16_t>>(ptd.pixlVals);
                const cv::Point *pPixlLocs = ptd.pixlLocs.data();

                int32_t n = 0;
                vcl::Vec16s sumVec(0);
                for (; n < regularSize; n += simdSize)
                {
                    for (int m=0; m<simdSize; ++m)
                    {
                        const int x = pPixlLocs->x + col;
                        const int y = pPixlLocs->y + row;
                        tempData[m] = pRows[y][x];
                        pPixlLocs += 1;
                    }

                    vcl::Vec16s tempVec0, tempVec1;
                    tempVec0.load(tempData.data());
                    tempVec1.load(pixlVals.data() + n);
                    sumVec += vcl::abs(tempVec0 - tempVec1);
                }

                int32_t partialSum = vcl::horizontal_add_x(sumVec);
                for (; n < numPoints; ++n)
                {
                    const int x = pPixlLocs->x + col;
                    const int y = pPixlLocs->y + row;
                    partialSum += std::abs(pixlVals[n] - pRows[y][x]);
                    pPixlLocs += 1;
                }

                int32_t sad = partialSum / numPoints;
                if (sad < minSAD)
                {
                    minSAD = sad;
                    minAng = ptd.angle;
                }
            }

            if (minSAD < topSad)
            {
                top_layer_angle_.at<float>(originPt)   = minAng;
                top_layer_score_.at<uint8_t>(originPt) = static_cast<uint8_t>(255-minSAD);
            }
        }
    }

    return SpamResult::kSR_OK;
}

SpamResult PixelTemplate::CreatePixelTemplate(const PixelTmplCreateData &createData)
{
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

        pyramid_tmpl_datas_.emplace_back(angleStep, 0.0);
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