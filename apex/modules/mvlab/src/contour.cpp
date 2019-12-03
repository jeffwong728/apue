#include "precomp.hpp"
#include "contour_impl.hpp"
#include "contour_array_impl.hpp"
#include "utility.hpp"
#include <boost/algorithm/string.hpp>

namespace cv {
namespace mvlab {

Ptr<Contour> Contour::GenEmpty()
{
    return makePtr<ContourImpl>();
}

Ptr<Contour> Contour::GenRectangle(const cv::Rect2f &rect)
{
    ScalablePoint2fSequence points;
    points.reserve(4);
    points.emplace_back(rect.tl());
    points.emplace_back(rect.x, rect.y + rect.height);
    points.emplace_back(rect.br());
    points.emplace_back(rect.x + rect.width, rect.y);
    return makePtr<ContourImpl>(&points, K_YES, true);
}

Ptr<Contour> Contour::GenRotatedRectangle(const cv::RotatedRect &rotatedRect)
{
    ScalablePoint2fSequence corners(4);
    rotatedRect.points(corners.data());
    return makePtr<ContourImpl>(&corners, K_YES, true);
}

Ptr<Contour> Contour::GenCircle(const cv::Point2f &center, const float radius, const float resolution, const cv::String &specification)
{
    if (radius < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radStep = resolution / radius;
    ScalablePoint2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, true);
}

Ptr<Contour> Contour::GenCircleSector(const cv::Point2f &center,
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
    ScalablePoint2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, !boost::contains(specification, "arc"));
}

Ptr<Contour> Contour::GenEllipse(const cv::Point2f &center, const cv::Size2f &size, const float resolution, const cv::String &specification)
{
    if (size.width < 0.f || size.height < 0.f || resolution < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    const float radius = std::min(size.width, size.height);
    const float radStep = resolution / radius;
    ScalablePoint2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, true);
}

Ptr<Contour> Contour::GenEllipseSector(const cv::Point2f &center,
    const cv::Size2f &size,
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
    ScalablePoint2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, !boost::contains(specification, "arc"));
}

Ptr<Contour> Contour::GenRotatedEllipse(const cv::Point2f &center,
    const cv::Size2f &size,
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
    ScalablePoint2fSequence corners(cvCeil(CV_2PI / radStep) + 2);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, true);
}

Ptr<Contour> Contour::GenRotatedEllipseSector(const cv::Point2f &center,
    const cv::Size2f &size,
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
    ScalablePoint2fSequence corners(cvCeil(angExt / Util::deg(radStep)) + 3);
    ScalablePoint2fSequence::pointer pCorner = corners.data();

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
    return makePtr<ContourImpl>(&corners, K_YES, !boost::contains(specification, "arc"));
}

Ptr<Contour> Contour::GenPolygon(const std::vector<cv::Point2f> &vertexes)
{
    return makePtr<ContourImpl>(vertexes, K_UNKNOWN, true);
}

Ptr<Contour> Contour::GenPolyline(const std::vector<cv::Point2f> &vertexes)
{
    return makePtr<ContourImpl>(vertexes, K_UNKNOWN, false);
}

Ptr<Contour> Contour::GenPolygonRounded(const std::vector<cv::Point2f> &vertexes,
    const std::vector<cv::Size2f> &radius,
    const float samplingInterval)
{
    if (vertexes.size()<3 || vertexes.size() != radius.size() || samplingInterval < 0.00001f)
    {
        return makePtr<ContourImpl>();
    }

    std::vector<cv::Point2f>::const_pointer p0 = vertexes.data() + vertexes.size() - 1;
    const std::vector<cv::Point2f>::const_pointer pend = vertexes.data() + vertexes.size();

    float len = 0.f;
    std::vector<cv::Point2f>::const_pointer p1 = vertexes.data();
    for (; p1 != pend; p0=p1, ++p1)
    {
        len += Util::dist(p0, p1);
    }

    ScalablePoint2fSequence corners;
    corners.reserve(cvCeil((len+ vertexes.size()*samplingInterval)/ samplingInterval) + vertexes.size());

    p0 = vertexes.data() + vertexes.size() - 2;
    p1 = vertexes.data() + vertexes.size() - 1;

    std::vector<cv::Size2f>::const_pointer  r2 = radius.data();
    std::vector<cv::Point2f>::const_pointer p2 = vertexes.data();
    std::vector<cv::Size2f>::const_pointer  r1 = radius.data() + radius.size() - 1;
    const float tol = samplingInterval * samplingInterval;
    const cv::Point2f *prev = p0;
    for (; p2 != pend; p0 = p1, p1 = p2, r1 = r2, ++p2, ++r2)
    {
        const cv::Point2f m0 = Util::midPoint(p0, p1);
        const cv::Point2f m1 = Util::midPoint(p1, p2);
        const float l0 = Util::dist(p0, p1) / 2;
        const float l1 = Util::dist(p1, p2) / 2;

        if (l0 > samplingInterval && l1 > samplingInterval)
        {
            const float t0 = std::max(0.f, std::min(l0, r1->width)) / l0;
            const float t1 = std::max(0.f, std::min(l1, r1->height)) / l0;

            const cv::Point2f pi = Util::interPoint(t0, p1, &m0);
            const cv::Point2f pf = Util::interPoint(t1, p1, &m1);

            if (Util::farPoint(&m0, prev, tol))
            {
                corners.emplace_back(m0);
                prev = &corners.back();
            }

            if (Util::farPoint(&pi, prev, tol))
            {
                corners.emplace_back(pi);
                prev = &corners.back();
            }

            Geom::QuadraticBezier crv(Geom::Point(pi.x, pi.y), Geom::Point(p1->x, p1->y), Geom::Point(pf.x, pf.y));
            const double clen = crv.length(0.001);
            if (clen > samplingInterval)
            {
                const double dt = samplingInterval / clen;
                for (double t = 0.0; t < 1.0; t += dt)
                {
                    const Geom::Point dpt = crv.pointAt(t);
                    const cv::Point2f fpt{static_cast<float>(dpt.x()), static_cast<float>(dpt.y())};
                    if (Util::farPoint(&fpt, prev, tol))
                    {
                        corners.emplace_back(fpt);
                        prev = &corners.back();
                    }
                }
            }

            if (Util::farPoint(&pf, prev, tol))
            {
                corners.emplace_back(pf);
                prev = &corners.back();
            }
        }
        else
        {
            if (Util::farPoint(p1, prev, tol))
            {
                corners.emplace_back(*p1);
                prev = &corners.back();
            }
        }
    }

    return makePtr<ContourImpl>(&corners, K_UNKNOWN, true);
}

Ptr<Contour> Contour::GenCross(const std::vector<cv::Point2f> &center, const std::vector<cv::Size2f> &size, const std::vector<float> &angle)
{
    if (center.size()==size.size() && center.size()==angle.size() && !center.empty())
    {
        std::vector<cv::Ptr<Contour>> contours(center.size());
        for (std::size_t i=0; i< center.size(); ++i)
        {
            ScalablePoint2fSequenceSequence curves(2);
            const cv::Point2f &cp = center[i];
            const float width  = size[i].width / 2;
            const float height = size[i].height / 2;
            const float ang = angle[i];
            curves[0].resize(2);
            curves[1].resize(2);
            curves[0][0] = cv::Point2f(cp.x + width * std::cosf(Util::rad(ang)), cp.y - width * std::sinf(Util::rad(ang)));
            curves[0][1] = cv::Point2f(cp.x + width * std::cosf(Util::rad(ang + 180)), cp.y - width * std::sinf(Util::rad(ang + 180)));
            curves[1][0] = cv::Point2f(cp.x + height * std::cosf(Util::rad(ang + 90)), cp.y - height * std::sinf(Util::rad(ang + 90)));
            curves[1][1] = cv::Point2f(cp.x + height * std::cosf(Util::rad(ang + 270)), cp.y - height * std::sinf(Util::rad(ang + 270)));

            contours[i] = makePtr<ContourImpl>(&curves, K_NO, false);
        }

        return makePtr<ContourArrayImpl>(&contours);
    }
    else
    {
        return makePtr<ContourImpl>();
    }
}

}
}
