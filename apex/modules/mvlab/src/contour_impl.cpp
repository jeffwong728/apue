#include "precomp.hpp"
#include "contour_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {
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

bool ContourImpl::Empty() const
{
    return vertexes_.empty();
}

int ContourImpl::Count() const
{
    return static_cast<int>(vertexes_.size());
}

double ContourImpl::Length() const
{
    if (length_ == boost::none)
    {
        double len = 0.;
        if (vertexes_.size() > 2)
        {
            const int numPoints = static_cast<int>(vertexes_.size()) - 1;
            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            len += Util::dist(vertexes_[0], vertexes_[numPoints]);

            int n = 0;
            const cv::Point2f *pt = vertexes_.data();
            for (; n < regularNumPoints; n += simdSize)
            {
                vcl::Vec8f v1, v2;
                v1.load(reinterpret_cast<const float *>(pt));
                v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
                vcl::Vec8f xprev = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                vcl::Vec8f yprev = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

                v1.load(reinterpret_cast<const float *>(pt + 1));
                v2.load(reinterpret_cast<const float *>(pt + 1 + simdSize / 2));
                vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);
                vcl::Vec8f dx = x - xprev;
                vcl::Vec8f dy = y - yprev;
                len += vcl::horizontal_add(vcl::sqrt(dx*dx+dy*dy));
            }

            for (; n < numPoints; ++n, ++pt)
            {
                len += Util::dist(pt, pt+1);
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
        AreaCenter();
    }

    return *area_;
}

cv::Point2d ContourImpl::Centroid() const
{
    if (centroid_ == boost::none)
    {
        AreaCenter();
    }

    return *centroid_;
}

cv::Rect ContourImpl::BoundingBox() const
{
    if (bbox_ == boost::none)
    {
        if (!vertexes_.empty())
        {
            const int numPoints = static_cast<int>(vertexes_.size());
            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            vcl::Vec8f top(std::numeric_limits<float>::max());
            vcl::Vec8f left(std::numeric_limits<float>::max());
            vcl::Vec8f bot(std::numeric_limits<float>::lowest());
            vcl::Vec8f right(std::numeric_limits<float>::lowest());

            int n = 0;
            const cv::Point2f *pt = vertexes_.data();
            for (; n < regularNumPoints; n += simdSize)
            {
                vcl::Vec8f v1, v2;
                v1.load(reinterpret_cast<const float *>(pt));
                v2.load(reinterpret_cast<const float *>(pt + simdSize/2));
                vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

                top   = vcl::min(top, x);
                left  = vcl::min(left, y);
                bot   = vcl::max(bot, x);
                right = vcl::max(right, y);

                pt += simdSize;
            }

            float xmin = vcl::horizontal_min(left);
            float xmax = vcl::horizontal_max(right);
            float ymin = vcl::horizontal_min(top);
            float ymax = vcl::horizontal_max(bot);
            for (; n < numPoints; ++n, ++pt)
            {
                xmin = std::min(xmin, pt->x);
                ymin = std::min(ymin, pt->y);
                xmax = std::max(xmax, pt->x);
                ymax = std::max(ymax, pt->y);
            }

            const int x = cvFloor(xmin);
            const int y = cvFloor(ymin);
            const int w = cvCeil(xmax) - x;
            const int h = cvCeil(ymax) - y;

            bbox_ = cv::Rect(x, y, w, h);
        }
        else
        {
            bbox_ = cv::Rect();
        }
    }

    return *bbox_;
}

Ptr<Contour> ContourImpl::Simplify(const float tolerance) const
{
    if (Empty())
    {
        return makePtr<ContourImpl>();
    }
    else
    {
        Point2fSequence approxCurve;
        cv::approxPolyDP(vertexes_, approxCurve, tolerance, is_closed_);
        return makePtr<ContourImpl>(&approxCurve, is_closed_);
    }
}

int ContourImpl::GetPoints(std::vector<Point2f> &points) const
{
    if (vertexes_.empty())
    {
        points.clear();
        return MLR_CONTOUR_EMPTY;
    }
    else
    {
        points.assign(vertexes_.cbegin(), vertexes_.cend());
        return MLR_SUCCESS;
    }
}

cv::Ptr<Contour> ContourImpl::Move(const cv::Point2f &delta) const
{
    Point2fSequence vertexes(vertexes_.cbegin(), vertexes_.cend());
    for (auto &v : vertexes)
    {
        v.x += delta.x;
        v.y += delta.y;
    }

    return makePtr<ContourImpl>(&vertexes, is_closed_);
}

cv::Ptr<Contour> ContourImpl::Zoom(const cv::Size2f &scale) const
{
    Point2fSequence vertexes(vertexes_.cbegin(), vertexes_.cend());
    for (auto &v : vertexes)
    {
        v.x *= scale.width;
        v.y *= scale.height;
    }

    return makePtr<ContourImpl>(&vertexes, is_closed_);
}

bool ContourImpl::TestClosed() const
{
    return is_closed_;
}

bool ContourImpl::TestPoint(const cv::Point2f &point) const
{
    return false;
}

bool ContourImpl::TestSelfIntersection() const
{
    return false;
}

void ContourImpl::Feed(Cairo::RefPtr<Cairo::Context> &cr) const
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

void ContourImpl::AreaCenter() const
{
    if (is_closed_ && vertexes_.size() > 2)
    {
        double area = 0;
        double cx = 0, cy = 0;
        const int numPoints = static_cast<int>(vertexes_.size()) - 1;
        constexpr int simdSize = 8;
        const int regularNumPoints = numPoints & (-simdSize);

        int n = 0;
        const cv::Point2f *pt = vertexes_.data();
        for (; n < regularNumPoints; n += simdSize)
        {
            vcl::Vec8f v1, v2;
            v1.load(reinterpret_cast<const float *>(pt));
            v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
            vcl::Vec8f xprev = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
            vcl::Vec8f yprev = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

            v1.load(reinterpret_cast<const float *>(pt + 1));
            v2.load(reinterpret_cast<const float *>(pt + 1 + simdSize / 2));
            vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
            vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);
            vcl::Vec8f dx = xprev * y;
            vcl::Vec8f dy = x * yprev;
            vcl::Vec8f a = dx - dy;
            area += vcl::horizontal_add(a);
            cx += vcl::horizontal_add((x + xprev)*a);
            cy += vcl::horizontal_add((y + yprev)*a);
            pt += simdSize;
        }

        for (; n <= numPoints; ++n)
        {
            const int n1 = (n == numPoints) ? 0 : (n + 1);
            const double a = vertexes_[n].x*vertexes_[n1].y - vertexes_[n1].x * vertexes_[n].y;
            area += a;
            cx += (vertexes_[n].x + vertexes_[n1].x)*a;
            cy += (vertexes_[n].y + vertexes_[n1].y)*a;
        }

        area_ = std::abs(area / 2);
        if (*area_ > 0.)
        {
            centroid_ = cv::Point2d(cx / (3 * area), cy / (3 * area));
        }
        else
        {
            centroid_ = cv::Point2d();
        }
    }
    else
    {
        area_ = 0;
        centroid_ = cv::Point2d();
    }
}

}
}
