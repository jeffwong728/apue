#include "precomp.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {

cv::Ptr<Region> Region::CreateEmpty()
{
    return makePtr<RegionImpl>();
}

cv::Ptr<Region> Region::CreateRectangle(const cv::Rect2f &rect)
{
    return makePtr<RegionImpl>(rect);
}

cv::Ptr<Region> Region::CreateRotatedRectangle(const cv::RotatedRect &rotatedRect)
{
    return makePtr<RegionImpl>(rotatedRect);
}

cv::Ptr<Region> Region::CreateCircle(const cv::Point2f &center, const float radius)
{
    return makePtr<RegionImpl>(center, radius);
}

cv::Ptr<Region> Region::CreateEllipse(const cv::Point2f &center, const cv::Size2f &size)
{
    return makePtr<RegionImpl>(center, size);
}

cv::Ptr<Region> Region::CreateRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float angle)
{
    return makePtr<RegionImpl>(center, size, angle);
}

cv::Ptr<Region> Region::CreatePolygon(const std::vector<cv::Point2f> &vertexes)
{
    return makePtr<RegionImpl>();
}

}
}
