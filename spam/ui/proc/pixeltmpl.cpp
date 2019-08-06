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
        return sr;
    }

    return SpamResult::kSR_OK;
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

    for (int l=0; l < createData.pyramidLevel; ++l)
    {
        const SpamRgn rgn(tmplRgns[l]);
        cv::Point anchorPoint{cvRound(rgn.Centroid().x), cvRound(rgn.Centroid().y)};

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

        const cv::Mat &pyrImg = pyrs_[l];
        for (double ang = 0; ang < createData.angleExtent; ang += angleStep)
        {
            const double deg = createData.angleStart + ang;
            SpamRgn originRgn(tmplRgns[l] * Geom::Translate(-anchorPoint.x, -anchorPoint.y)* Geom::Rotate::from_degrees(deg));

            PointSet pointSetOrigin(originRgn);
            PointSet pointSetTmpl(originRgn, anchorPoint);

            if (!pointSetTmpl.IsInsideImage(cv::Size(pyrImg.cols, pyrImg.rows)))
            {
                return SpamResult::kSR_TM_TEMPL_REGION_OUT_OF_RANGE;
            }

            std::vector<cv::Point> pixlLocs;
            std::vector<float>     pixlVals;
            const int numPoints = static_cast<int>(pointSetTmpl.size());
            for (int n=0; n<numPoints; ++n)
            {

            }
        }
    }

    return SpamResult::kSR_OK;
}