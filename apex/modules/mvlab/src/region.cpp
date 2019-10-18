#include "precomp.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {

Ptr<Region> Region::CreateEmpty()
{
    return makePtr<RegionImpl>();
}

Ptr<Region> Region::CreateRectangle(const Rect2f &rect)
{
    return makePtr<RegionImpl>(rect);
}

Ptr<Region> Region::CreateRotatedRectangle(const RotatedRect &rotatedRect)
{
    return makePtr<RegionImpl>(rotatedRect);
}

Ptr<Region> Region::CreateCircle(const Point2f &center, const float radius)
{
    return makePtr<RegionImpl>(center, radius);
}

Ptr<Region> Region::CreateEllipse(const Point2f &center, const Size2f &size)
{
    return makePtr<RegionImpl>(center, size);
}

Ptr<Region> Region::CreateRotatedEllipse(const Point2f &center, const Size2f &size, const float angle)
{
    return makePtr<RegionImpl>(center, size, angle);
}

}
}
