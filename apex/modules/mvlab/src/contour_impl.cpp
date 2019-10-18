#include "precomp.hpp"
#include "contour_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

ContourImpl::ContourImpl(const Rect2f &rect)
    : Contour()
    , is_closed_(true)
    , path_(Geom::Rect(Geom::Point(rect.tl().x, rect.tl().y), Geom::Point(rect.br().x, rect.br().y)))
{
}

ContourImpl::ContourImpl(const RotatedRect &rotatedRect)
    : Contour()
    , is_closed_(true)
{
    Point2f corners[4];
    rotatedRect.points(corners);

    Geom::PathVector pv;
    Geom::PathBuilder pb(pv);
    pb.moveTo(Geom::Point(corners[0].x, corners[0].y));
    pb.lineTo(Geom::Point(corners[1].x, corners[1].y));
    pb.lineTo(Geom::Point(corners[2].x, corners[2].y));
    pb.lineTo(Geom::Point(corners[3].x, corners[3].y));
    pb.closePath();

    path_.swap(pv[0]);
}

ContourImpl::ContourImpl(const Point2f &center, const float radius)
    : Contour()
    , is_closed_(true)
    , path_(Geom::Circle(center.x, center.y, radius))
{
}

ContourImpl::ContourImpl(const Point2f &center, const Size2f &size)
    : Contour()
    , is_closed_(true)
    , path_(Geom::Ellipse(center.x, center.y, size.width, size.height, 0.))
{
}

ContourImpl::ContourImpl(const Point2f &center, const Size2f &size, const float angle)
    : Contour()
    , is_closed_(true)
    , path_(Geom::Ellipse(center.x, center.y, size.width, size.height, angle))
{
}

ContourImpl::ContourImpl(const Geom::Path &path, const bool closed)
    : Contour()
    , is_closed_(closed)
    , path_(path)
{
}

int ContourImpl::Draw(Mat &img, const Scalar& color, float thickness, int style) const
{
    if (img.empty())
    {
        const Rect bbox = ContourImpl::BoundingBox();
        if (bbox.width > 0 && bbox.height > 0)
        {
            img = Mat::ones(bbox.br().y + 1, bbox.br().x + 1, CV_8UC4) * 255;
        }
        else
        {
            return MLR_REGION_EMPTY;
        }
    }

    if (img.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    int dph = img.depth();
    int cnl = img.channels();
    if (CV_8U == dph && 4 == cnl)
    {
        auto imgSurf = Cairo::ImageSurface::create(img.data, Cairo::Format::FORMAT_RGB24, img.cols, img.rows, static_cast<int>(img.step1()));
        auto cr = Cairo::Context::create(imgSurf);
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(path_);
        cr->set_line_width(thickness);
        cr->set_source_rgba(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, color[3] / 255.0);
        cr->stroke();
    }

    return MLR_SUCCESS;
}

int ContourImpl::Draw(InputOutputArray img, const Scalar& color, float thickness, int style) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int res = ContourImpl::Draw(imgMat, color, thickness, style);
        img.assign(imgMat);
        return res;
    }
    else
    {
        return ContourImpl::Draw(imgMat, color, thickness, style);
    }
}

double ContourImpl::Length() const
{
    if (length_ == boost::none)
    {
        double len = 0.;
        if (!path_.empty())
        {
            for (const Geom::Curve &curve : path_)
            {
                len += curve.length();
            }
        }

        length_ = len;
    }

    return *length_;
}

double ContourImpl::Area() const
{
    if (area_ == boost::none)
    {
        double area = 0.0;
        Geom::Point centroid;
        if (is_closed_ && !path_.empty())
        {
            Geom::centroid(Geom::paths_to_pw(Geom::PathVector(path_)), centroid, area);
        }

        area_ = std::abs(area);
        centroid_.emplace(centroid.x(), centroid.y());
    }

    return *area_;
}

cv::Point2d ContourImpl::Centroid() const
{
    if (centroid_ == boost::none)
    {
        Area();
    }

    return *centroid_;
}

Rect ContourImpl::BoundingBox() const
{
    if (bbox_ == boost::none)
    {
        cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
        cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

        cv::Rect rect;
        if (!path_.empty())
        {
            Geom::OptRect oRect = path_.boundsFast();
            if (oRect)
            {
                
                rect.x = cvFloor(oRect->left());
                rect.y = cvFloor(oRect->top());
                rect.width = cvCeil(oRect->width());
                rect.height = cvCeil(oRect->height());
            }
        }

        bbox_ = rect;
    }

    return *bbox_;
}

void ContourImpl::ClearCacheData()
{
    length_   = boost::none;
    bbox_     = boost::none;
    area_     = boost::none;
    centroid_ = boost::none;
    bbox_     = boost::none;
    start_    = boost::none;
    points_   = boost::none;
}

}
}
