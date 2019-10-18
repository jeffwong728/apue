#include "precomp.hpp"
#include "contour_impl.hpp"

namespace cv {
namespace mvlab {

Ptr<Contour> Contour::CreateEmpty()
{
    return makePtr<ContourImpl>();
}

Ptr<Contour> Contour::CreateRectangle(const Rect2f &rect)
{
    return makePtr<ContourImpl>(rect);
}

Ptr<Contour> Contour::CreateRotatedRectangle(const RotatedRect &rotatedRect)
{
    return makePtr<ContourImpl>(rotatedRect);
}

Ptr<Contour> Contour::CreateCircle(const Point2f &center, const float radius)
{
    return makePtr<ContourImpl>(center, radius);
}

Ptr<Contour> Contour::CreateEllipse(const Point2f &center, const Size2f &size)
{
    return makePtr<ContourImpl>(center, size);
}

Ptr<Contour> Contour::CreateRotatedEllipse(const Point2f &center, const Size2f &size, const float angle)
{
    return makePtr<ContourImpl>(center, size, -angle);
}

}
}
