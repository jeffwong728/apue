#include "precomp.hpp"
#include "contour_impl.hpp"

namespace cv {
namespace mvlab {

Ptr<Contour> Contour::GenEmpty()
{
    return makePtr<ContourImpl>();
}

Ptr<Contour> Contour::GenRectangle(const Rect2f &rect)
{
    Point2fSequence points;
    points.reserve(4);
    points.emplace_back(rect.tl());
    points.emplace_back(rect.x, rect.y + rect.height);
    points.emplace_back(rect.br());
    points.emplace_back(rect.x + rect.width, rect.y);
    return makePtr<ContourImpl>(&points, true);
}

Ptr<Contour> Contour::GenRotatedRectangle(const RotatedRect &rotatedRect)
{
    Point2fSequence corners(4);
    rotatedRect.points(corners.data());
    return makePtr<ContourImpl>(&corners, true);
}

Ptr<Contour> Contour::GenCircle(const Point2f &center, const float radius, const float resolution, const cv::String &pointOrder)
{
    if (radius < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    Point2fSequence::pointer pCorner = corners.data();

    if (pointOrder == "negative")
    {
        for (float a = CV_2PI; a>0.f; a -= radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }
    }
    else
    {
        for (float a = 0.f; a < CV_2PI; a += radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(center, radius, &corners);
}

Ptr<Contour> Contour::GenEllipse(const Point2f &center, const Size2f &size)
{
    return makePtr<ContourImpl>(center, size);
}

Ptr<Contour> Contour::GenRotatedEllipse(const Point2f &center, const Size2f &size, const float angle)
{
    return makePtr<ContourImpl>(center, size, -angle);
}

Ptr<Contour> Contour::GenPolygon(const std::vector<Point2f> &vertexes)
{
    return makePtr<ContourImpl>(vertexes, true);
}

Ptr<Contour> Contour::GenPolyline(const std::vector<Point2f> &vertexes)
{
    return makePtr<ContourImpl>(vertexes, false);
}

}
}
