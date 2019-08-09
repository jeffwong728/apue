#include "pixeltmpl.h"
#include "basic.h"
#include <limits>
#include <stack>
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
    cv::buildPyramid(tmplImg, pyrs_, createData.pyramidLevel, cv::BORDER_REFLECT);
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

            ltl.tmplDatas.emplace_back(deg, 1.0);
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

            ptd.bbox = pixlLocs.BoundingBox();
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