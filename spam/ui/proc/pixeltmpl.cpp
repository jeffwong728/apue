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
        return SpamResult::kSR_IMAGE_EMPTY;
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

    if (createData.angleRange.start > createData.angleRange.end)
    {
        return SpamResult::kSR_TM_ANGLE_RANGE_INVALID;
    }

    if (createData.angleRange.start < -180 || createData.angleRange.start > 180)
    {
        return SpamResult::kSR_TM_ANGLE_RANGE_INVALID;
    }

    if (createData.angleRange.end < -180 || createData.angleRange.end > 180)
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

    for (int l=0; l < createData.pyramidLevel; ++l)
    {
        SpamRgn rgn(tmplRgns[l]);
        cv::Point anchorPoint{cvRound(rgn.Centroid().x), cvRound(rgn.Centroid().y)};

        SpamRgn originRgn(tmplRgns[l]*Geom::Translate(anchorPoint.x, anchorPoint.y));

        float radius = 0;
        cv::Point2f center;
        std::vector<cv::Point> points;
        cv::minEnclosingCircle(points, center, radius);
    }

    return SpamResult::kSR_OK;
}