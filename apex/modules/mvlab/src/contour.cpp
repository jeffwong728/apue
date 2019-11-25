#include "precomp.hpp"
#include "contour_impl.hpp"
#include "utility.hpp"
#include <boost/algorithm/string.hpp>

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

Ptr<Contour> Contour::GenCircle(const Point2f &center, const float radius, const float resolution, const cv::String &specification)
{
    if (radius < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    Point2fSequence::pointer pCorner = corners.data();

    if (boost::contains(specification, "negative"))
    {
        for (float a = static_cast<float>(CV_2PI); a>0.f; a -= radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }
    }
    else
    {
        for (float a = 0.f; a < static_cast<float>(CV_2PI); a += radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(center, radius, &corners);
}

Ptr<Contour> Contour::GenCircleSector(const Point2f &center,
    const float radius,
    const float startAngle,
    const float endAngle,
    const float resolution,
    const cv::String &specification)
{
    const float angBeg = Util::constrainAngle(startAngle);
    const float angEnd = Util::constrainAngle(endAngle);
    const float angExt = (angBeg < angEnd) ? (angEnd - angBeg) : (360.f - angBeg + angEnd);

    if (radius < 0.f || resolution < 0.00001f || angExt < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    Point2fSequence::pointer pCorner = corners.data();

    if (boost::contains(specification, "negative"))
    {
        for (float a = Util::rad(angEnd); a > Util::rad(angEnd - angExt); a -= radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }

        pCorner->x = center.x + radius * std::cosf(Util::rad(angBeg));
        pCorner->y = center.y - radius * std::sinf(Util::rad(angBeg));
        ++pCorner;
    }
    else
    {
        for (float a = Util::rad(angBeg); a < Util::rad(angBeg + angExt); a += radStep)
        {
            pCorner->x = center.x + radius * std::cosf(a);
            pCorner->y = center.y - radius * std::sinf(a);
            ++pCorner;
        }

        pCorner->x = center.x + radius * std::cosf(Util::rad(angEnd));
        pCorner->y = center.y - radius * std::sinf(Util::rad(angEnd));
        ++pCorner;
    }

    if (boost::contains(specification, "slice"))
    {
        pCorner->x = center.x;
        pCorner->y = center.y;
        ++pCorner;
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(&corners, !boost::contains(specification, "arc"));
}

Ptr<Contour> Contour::GenEllipse(const Point2f &center, const Size2f &size, const float resolution, const cv::String &specification)
{
    if (size.width < 0.f || size.height < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radius = std::min(size.width, size.height);
    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    Point2fSequence::pointer pCorner = corners.data();

    if (specification == "negative")
    {
        for (float a = static_cast<float>(CV_2PI); a > 0.f; a -= radStep)
        {
            pCorner->x = center.x + size.width * std::cosf(a);
            pCorner->y = center.y - size.height * std::sinf(a);
            ++pCorner;
        }
    }
    else
    {
        for (float a = 0.f; a < CV_2PI; a += radStep)
        {
            pCorner->x = center.x + size.width * std::cosf(a);
            pCorner->y = center.y - size.height * std::sinf(a);
            ++pCorner;
        }
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(center, size, &corners);
}

Ptr<Contour> Contour::GenEllipseSector(const Point2f &center,
    const Size2f &size,
    const float startAngle,
    const float endAngle,
    const float resolution,
    const cv::String &specification)
{
    const float radius = std::min(size.width, size.height);
    const float angBeg = Util::constrainAngle(startAngle);
    const float angEnd = Util::constrainAngle(endAngle);
    const float angExt = (angBeg < angEnd) ? (angEnd - angBeg) : (360.f - angBeg + angEnd);

    if (radius < 0.f || resolution < 0.00001f || angExt < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    Point2fSequence::pointer pCorner = corners.data();

    if (boost::contains(specification, "negative"))
    {
        for (float a = Util::rad(angEnd); a > Util::rad(angEnd - angExt); a -= radStep)
        {
            pCorner->x = center.x + size.width * std::cosf(a);
            pCorner->y = center.y - size.height * std::sinf(a);
            ++pCorner;
        }

        pCorner->x = center.x + size.width * std::cosf(Util::rad(angBeg));
        pCorner->y = center.y - size.height * std::sinf(Util::rad(angBeg));
        ++pCorner;
    }
    else
    {
        for (float a = Util::rad(angBeg); a < Util::rad(angBeg + angExt); a += radStep)
        {
            pCorner->x = center.x + size.width * std::cosf(a);
            pCorner->y = center.y - size.height * std::sinf(a);
            ++pCorner;
        }

        pCorner->x = center.x + size.width * std::cosf(Util::rad(angEnd));
        pCorner->y = center.y - size.height * std::sinf(Util::rad(angEnd));
        ++pCorner;
    }

    if (boost::contains(specification, "slice"))
    {
        pCorner->x = center.x;
        pCorner->y = center.y;
        ++pCorner;
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(&corners, !boost::contains(specification, "arc"));
}

Ptr<Contour> Contour::GenRotatedEllipse(const Point2f &center,
    const Size2f &size,
    const float angle,
    const float resolution,
    const cv::String &specification)
{
    if (size.width < 0.f || size.height < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radius = std::min(size.width, size.height);
    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    Point2fSequence::pointer pCorner = corners.data();

    const float alpha = std::cosf(Util::rad(angle));
    const float beta  = std::sinf(Util::rad(angle));

    if (specification == "negative")
    {
        for (float a = static_cast<float>(CV_2PI); a > 0.f; a -= radStep)
        {
            const float x = size.width * std::cosf(a);
            const float y = size.height * std::sinf(a);
            pCorner->x = center.x + x * alpha - y * beta;
            pCorner->y = center.y - x * beta - y * alpha;
            ++pCorner;
        }
    }
    else
    {
        for (float a = 0.f; a < static_cast<float>(CV_2PI); a += radStep)
        {
            const float x = size.width * std::cosf(a);
            const float y = size.height * std::sinf(a);
            pCorner->x = center.x + x * alpha - y * beta;
            pCorner->y = center.y - x * beta - y * alpha;
            ++pCorner;
        }
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(center, size, angle, &corners);
}

Ptr<Contour> Contour::GenRotatedEllipseSector(const Point2f &center,
    const Size2f &size,
    const float angle,
    const float startAngle,
    const float endAngle,
    const float resolution,
    const cv::String &specification)
{
    const float radius = std::min(size.width, size.height);
    const float angBeg = Util::constrainAngle(startAngle);
    const float angEnd = Util::constrainAngle(endAngle);
    const float angExt = (angBeg < angEnd) ? (angEnd - angBeg) : (360.f - angBeg + angEnd);

    if (radius < 0.f || resolution < 0.00001f || angExt < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radStep = resolution / radius;
    Point2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    Point2fSequence::pointer pCorner = corners.data();

    const float alpha = std::cosf(Util::rad(angle));
    const float beta = std::sinf(Util::rad(angle));

    if (boost::contains(specification, "negative"))
    {
        for (float a = Util::rad(angEnd); a > Util::rad(angEnd - angExt); a -= radStep)
        {
            const float x = size.width * std::cosf(a);
            const float y = size.height * std::sinf(a);
            pCorner->x = center.x + x * alpha - y * beta;
            pCorner->y = center.y - x * beta - y * alpha;
            ++pCorner;
        }

        const float x = size.width * std::cosf(Util::rad(angBeg));
        const float y = size.height * std::sinf(Util::rad(angBeg));
        pCorner->x = center.x + x * alpha - y * beta;
        pCorner->y = center.y - x * beta - y * alpha;
        ++pCorner;
    }
    else
    {
        for (float a = Util::rad(angBeg); a < Util::rad(angBeg + angExt); a += radStep)
        {
            const float x = size.width * std::cosf(a);
            const float y = size.height * std::sinf(a);
            pCorner->x = center.x + x * alpha - y * beta;
            pCorner->y = center.y - x * beta - y * alpha;
            ++pCorner;
        }

        const float x = size.width * std::cosf(Util::rad(angEnd));
        const float y = size.height * std::sinf(Util::rad(angEnd));
        pCorner->x = center.x + x * alpha - y * beta;
        pCorner->y = center.y - x * beta - y * alpha;
        ++pCorner;
    }

    if (boost::contains(specification, "slice"))
    {
        pCorner->x = center.x;
        pCorner->y = center.y;
        ++pCorner;
    }

    corners.resize(std::distance(corners.data(), pCorner));
    return makePtr<ContourImpl>(&corners, !boost::contains(specification, "arc"));
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
