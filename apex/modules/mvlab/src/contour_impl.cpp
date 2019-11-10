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

    const_cast<boost::optional<Geom::Path>&>(path_).emplace(pv[0]);
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

ContourImpl::ContourImpl(const std::vector<Point2f> &vertexes, const bool closed)
    : Contour()
    , is_closed_(closed)
    , vertexes_(vertexes.cbegin(), vertexes.cend())
{
}

ContourImpl::ContourImpl(Point2fSequence *vertexes, const bool closed)
    : Contour()
    , is_closed_(closed)
    , vertexes_(std::move(*vertexes))
{
}

int ContourImpl::Draw(Mat &img, const Scalar& color, const float thickness, const int style) const
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
        DrawVerified(img, color, thickness, style);
    }
    else if (CV_8U == dph && 3 == cnl)
    {
        Mat colorImg;
        cvtColor(img, colorImg, cv::COLOR_BGR2BGRA);
        DrawVerified(colorImg, color, thickness, style);
        cvtColor(colorImg, img, cv::COLOR_BGRA2BGR);
    }
    else if (CV_8U == dph && 1 == cnl)
    {
        Mat colorImg;
        cvtColor(img, colorImg, cv::COLOR_GRAY2BGRA);
        DrawVerified(colorImg, color, thickness, style);
        cvtColor(colorImg, img, cv::COLOR_BGRA2GRAY);
    }
    else
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    return MLR_SUCCESS;
}

int ContourImpl::Draw(InputOutputArray img, const Scalar& color, const float thickness, const int style) const
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
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = ContourImpl::Draw(imgMat, color, thickness, style);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

double ContourImpl::Length() const
{
    if (length_ == boost::none)
    {
        double len = 0.;
        if (!path_->empty())
        {
            for (const Geom::Curve &curve : *path_)
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
        if (is_closed_ && !path_->empty())
        {
            Geom::centroid(Geom::paths_to_pw(Geom::PathVector(*path_)), centroid, area);
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
        if (!path_->empty())
        {
            Geom::OptRect oRect = path_->boundsFast();
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

int ContourImpl::Simplify(const float tolerance, std::vector<Point2f> &vertexes) const
{
    vertexes.assign(vertexes_.cbegin(), vertexes_.cend());
    return MLR_SUCCESS;
}

void ContourImpl::Feed(Cairo::RefPtr<Cairo::Context> &cr) const
{
    if (GetPath().empty())
    {
        const auto &vertexes = GetVertexes();
        const int numVertexes = static_cast<int>(vertexes.size());
        if (numVertexes > 1)
        {
            cr->move_to(vertexes.front().x, vertexes.front().y);
            for (int i = 1; i < numVertexes; ++i)
            {
                cr->line_to(vertexes[i].x, vertexes[i].y);
            }
            if (is_closed_)
            {
                cr->close_path();
            }
        }
    }
    else
    {
        Geom::CairoPathSink cairoPathSink(cr->cobj());
        cairoPathSink.feed(GetPath());
    }
}

void ContourImpl::ClearCacheData()
{
    length_   = boost::none;
    bbox_     = boost::none;
    area_     = boost::none;
    centroid_ = boost::none;
    bbox_     = boost::none;
    start_    = boost::none;
}

void ContourImpl::DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const
{
    auto imgSurf = Cairo::ImageSurface::create(img.data, Cairo::Format::FORMAT_RGB24, img.cols, img.rows, static_cast<int>(img.step1()));
    auto cr = Cairo::Context::create(imgSurf);

    std::vector<double> dashes = Util::GetDashesPattern(style, thickness);
    if (!dashes.empty())
    {
        cr->set_dash(dashes, 0.);
    }

    Feed(cr);

    cr->set_line_width(thickness);
    cr->set_source_rgba(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, color[3] / 255.0);
    cr->stroke();
}

}
}
