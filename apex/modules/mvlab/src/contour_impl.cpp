#include "precomp.hpp"
#include "contour_impl.hpp"
#include "utility.hpp"
#include "convex_hull.hpp"
#include "rot_calipers.hpp"
#include "Miniball.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {
ContourImpl::ContourImpl(const std::vector<Point2f> &vertexes, const int isSimple, const bool closed)
    : Contour()
    , is_closed_(closed)
    , is_simple_(isSimple)
    , is_convex_(K_UNKNOWN)
    , curves_(1, ScalablePoint2fSequence(vertexes.cbegin(), vertexes.cend()))
{
}

ContourImpl::ContourImpl(ScalablePoint2fSequence *vertexes, const int isSimple, const bool closed)
    : Contour()
    , is_closed_(closed)
    , is_simple_(isSimple)
    , is_convex_(K_UNKNOWN)
    , curves_(1)
{
    vertexes->swap(const_cast<ScalablePoint2fSequence &>(curves_.front()));
}

ContourImpl::ContourImpl(ScalablePoint2fSequenceSequence *curves, const int isSimple, const bool closed)
    : Contour()
    , is_closed_(closed)
    , is_simple_(isSimple)
    , is_convex_(K_UNKNOWN)
    , curves_(std::move(*curves))
{
}

int ContourImpl::Draw(Mat &img, const Scalar& color, const float thickness, const int style) const
{
    if (img.empty())
    {
        std::vector<cv::Rect> boundingBoxes;
        ContourImpl::GetBoundingBox(boundingBoxes);

        Rect bbox;
        for (const cv::Rect &box : boundingBoxes)
        {
            bbox |= box;
        }

        if (bbox.width > 0 && bbox.height > 0)
        {
            img = Mat::ones(bbox.br().y + 1, bbox.br().x + 1, CV_8UC4) * 255;
        }
        else
        {
            return MLR_CONTOUR_EMPTY;
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
    return curves_.empty() || std::all_of(curves_.cbegin(), curves_.cend(), [](const ScalablePoint2fSequence &item) { return item.empty(); });
}

int ContourImpl::Count() const
{
    return ContourImpl::Empty()? 0 : 1;
}

void ContourImpl::GetCountPoints(std::vector<int> &cPoints) const
{
    cPoints.resize(0);
    cPoints.push_back(ContourImpl::CountPoints());
}

void ContourImpl::GetLength(std::vector<double> &lengthes) const
{
    lengthes.resize(0);
    lengthes.push_back(ContourImpl::Length());
}

void ContourImpl::GetArea(std::vector<double> &areas) const
{
    areas.resize(0);
    areas.push_back(ContourImpl::Area());
}

void ContourImpl::GetCentroid(std::vector<cv::Point2f> &centroids) const
{
    centroids.resize(0);
    centroids.push_back(ContourImpl::Centroid());
}

void ContourImpl::GetBoundingBox(std::vector<cv::Rect> &boundingBoxes) const
{
    boundingBoxes.resize(0);
    boundingBoxes.push_back(ContourImpl::BoundingBox());
}

void ContourImpl::GetSmallestCircle(std::vector< cv::Point3d> &miniCircles) const
{
    miniCircles.resize(0);
    miniCircles.push_back(ContourImpl::SmallestCircle());
}

void ContourImpl::GetCircularity(std::vector<double> &circularities) const
{
    circularities.resize(0);
    circularities.push_back(ContourImpl::Circularity());
}

Ptr<Contour> ContourImpl::Simplify(const float tolerance) const
{
    if (Empty())
    {
        return makePtr<ContourImpl>();
    }
    else
    {
        ScalablePoint2fSequenceSequence approxCurves(curves_.size());
        auto itCurve  = approxCurves.begin();
        for (const auto &c : curves_)
        {
            Point2fSequence approxCurve;
            cv::approxPolyDP(Point2fSequence(c.cbegin(), c.cend()), approxCurve, tolerance, is_closed_);
            ScalablePoint2fSequence tPoints(approxCurve.cbegin(), approxCurve.cend());
            itCurve->swap(tPoints);
            ++itCurve;
        }

        return makePtr<ContourImpl>(&approxCurves, is_simple_, is_closed_);
    }
}

cv::Ptr<Contour> ContourImpl::GetConvex() const
{
    if (!Empty())
    {
        if (ContourImpl::TestConvex())
        {
            return (cv::Ptr<ContourImpl>)const_cast<ContourImpl *>(this)->shared_from_this();
        }
        else
        {
            if (!convex_hull_)
            {
                convex_hull_ = GetConvexImpl();
                cv::Ptr<ContourImpl> contImp = convex_hull_.dynamicCast<ContourImpl>();
                contImp->is_simple_ = K_YES;
                contImp->is_convex_ = K_YES;
            }
            return convex_hull_;
        }
    }

    return makePtr<ContourImpl>();
}

int ContourImpl::GetPoints(std::vector<Point2f> &points) const
{
    if (curves_.empty())
    {
        points.clear();
        return MLR_CONTOUR_EMPTY;
    }
    else
    {
        points.assign(curves_.front().cbegin(), curves_.front().cend());
        return MLR_SUCCESS;
    }
}

cv::Ptr<Contour> ContourImpl::Move(const cv::Point2f &delta) const
{
    ScalablePoint2fSequenceSequence curves(curves_.size());
    auto itCurve = curves.begin();
    for (const auto &c : curves_)
    {
        ScalablePoint2fSequence vertexes(c.cbegin(), c.cend());
        for (auto &v : vertexes)
        {
            v.x += delta.x;
            v.y += delta.y;
        }
        itCurve->swap(vertexes);
        ++itCurve;
    }

    return makePtr<ContourImpl>(&curves, is_simple_, is_closed_);
}

cv::Ptr<Contour> ContourImpl::Zoom(const cv::Size2f &scale) const
{
    ScalablePoint2fSequenceSequence curves(curves_.size());
    auto itCurve = curves.begin();
    for (const auto &c : curves_)
    {
        ScalablePoint2fSequence vertexes(c.cbegin(), c.cend());
        for (auto &v : vertexes)
        {
            v.x *= scale.width;
            v.y *= scale.height;
        }
        itCurve->swap(vertexes);
        ++itCurve;
    }

    return makePtr<ContourImpl>(&curves, is_simple_, is_closed_);
}

cv::Ptr<Contour> ContourImpl::AffineTrans(const cv::Matx33d &homoMat2D) const
{
    const cv::Matx33d m0 = HomoMat2d::Translate(homoMat2D, cv::Point2d(0.5, 0.5));
    const cv::Matx33d m1 = HomoMat2d::TranslateLocal(m0, cv::Point2d(-0.5, -0.5));
    ScalablePoint2fSequenceSequence curves(curves_.size());
    auto itCurve = curves.begin();
    for (const auto &c : curves_)
    {
        ScalablePoint2fSequence vertexes(c.cbegin(), c.cend());
        for (auto &v : vertexes)
        {
            const float x = v.x;
            const float y = v.y;
            v.x = static_cast<float>(m1.val[0] * x + m1.val[1] * y + m1.val[2]);
            v.y = static_cast<float>(m1.val[3] * x + m1.val[4] * y + m1.val[5]);
        }
        itCurve->swap(vertexes);
        ++itCurve;
    }

    return makePtr<ContourImpl>(&curves, is_simple_, is_closed_);
}

void ContourImpl::GetTestClosed(std::vector<int> &isClosed) const
{
    isClosed.resize(0);
    isClosed.push_back(is_closed_);
}

void ContourImpl::GetTestConvex(std::vector<int> &isConvex) const
{
    isConvex.resize(0);
    isConvex.push_back(ContourImpl::TestConvex());
}

void ContourImpl::GetTestPoint(const cv::Point2f &point, std::vector<int> &isInside) const
{
    isInside.resize(0);
    isInside.push_back(ContourImpl::TestPoint(point));
}

void ContourImpl::GetTestSelfIntersection(const cv::String &/*closeContour*/, std::vector<int> &doesIntersect) const
{
    doesIntersect.clear();
}

void ContourImpl::Feed(Cairo::RefPtr<Cairo::Context> &cr) const
{
    for (const auto &vertexes : curves_)
    {
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
    if (is_closed_ && !curves_.empty())
    {
        const auto &vertexes = curves_.front();
        if (vertexes.size() > 2)
        {
            double area = 0;
            double cx = 0, cy = 0;
            const int numPoints = static_cast<int>(vertexes.size()) - 1;
            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            int n = 0;
            const cv::Point2f *pt = vertexes.data();
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
                const double a = vertexes[n].x*vertexes[n1].y - vertexes[n1].x * vertexes[n].y;
                area += a;
                cx += (vertexes[n].x + vertexes[n1].x)*a;
                cy += (vertexes[n].y + vertexes[n1].y)*a;
            }

            area_ = area;
            if (std::abs(area) > 0.)
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
            for (const cv::Point2f &v : vertexes)
            {
                centroid_->x += v.x;
                centroid_->y += v.y;
            }
            centroid_->x /= static_cast<int>(vertexes.size());
            centroid_->y /= static_cast<int>(vertexes.size());
        }
    }
    else
    {
        area_ = 0.;
        centroid_ = cv::Point2d();
    }
}

void ContourImpl::ChangedCoordinatesToFixed() const
{
    UScalableIntSequence x_fixed_;
    UScalableIntSequence y_fixed_;
    if (!curves_.empty() && x_fixed_.empty())
    {
        constexpr int simdSize = 8;
        const int numPoints = static_cast<int>(curves_.size());
        const int regularNumPoints = numPoints & (-simdSize);

        x_fixed_.resize(curves_.size()+1);
        y_fixed_.resize(curves_.size()+1);

        int n = 0;
        const cv::Point2f *pt = curves_.front().data();
        int *px = x_fixed_.data();
        int *py = y_fixed_.data();
        vcl::Vec8f a(F_XY_ONE);
        for (; n < regularNumPoints; n += simdSize)
        {
            vcl::Vec8f v1, v2;
            v1.load(reinterpret_cast<const float *>(pt));
            v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
            vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
            vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);
            vcl::Vec8i ix = vcl::roundi(x*a);
            vcl::Vec8i iy = vcl::roundi(y*a);
            ix.store(px);
            iy.store(py);
            pt += simdSize;
            px += simdSize;
            py += simdSize;
        }

        for (; n < numPoints; ++n, ++pt, ++px, ++py)
        {
            *px = cvRound(pt->x * F_XY_ONE);
            *py = cvRound(pt->y * F_XY_ONE);
        }

        x_fixed_.back() = x_fixed_.front();
        y_fixed_.back() = y_fixed_.front();
    }
}

cv::Ptr<Contour> ContourImpl::GetConvexImpl() const
{
    cv::String optVal;
    GetGlobalOption("convex_hull_method", optVal);
    if (1 == curves_.size())
    {
        if ("Melkman" == optVal)
        {
            ScalablePoint2fSequence tPoints = ConvexHull::MelkmanSimpleHull(curves_.front().data(), static_cast<int>(curves_.front().size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_YES, true);
        }
        else if ("Sklansky" == optVal)
        {
            ScalablePoint2fSequence tPoints = ConvexHull::Sklansky(curves_.front().data(), static_cast<int>(curves_.front().size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_YES, true);
        }
        else
        {
            ScalablePoint2fSequence tPoints = ConvexHull::AndrewMonotoneChain(curves_.front().data(), static_cast<int>(curves_.front().size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_YES, true);
        }
    }
    else
    {
        std::size_t numPoints = 0;
        for (const auto &c : curves_)
        {
            numPoints += c.size();
        }

        UScalablePoint2fSequence points(numPoints);
        UScalablePoint2fSequence::pointer pDst = points.data();
        for (const auto &c : curves_)
        {
            std::memcpy(pDst, c.data(), c.size() * sizeof(ScalablePoint2fSequence::value_type));
            pDst += c.size();
        }

        if ("Sklansky" == optVal)
        {
            ScalablePoint2fSequence tPoints = ConvexHull::Sklansky(points.data(), static_cast<int>(points.size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_YES, true);
        }
        else
        {
            ScalablePoint2fSequence tPoints = ConvexHull::AndrewMonotoneChain(points.data(), static_cast<int>(points.size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_YES, true);
        }
    }
}

int ContourImpl::CountPoints() const
{
    int cPoints = 0;
    for (const auto &vertexes : curves_)
    {
        cPoints += static_cast<int>(vertexes.size());
    }
    return cPoints;
}

double ContourImpl::Area() const
{
    if (boost::none == area_)
    {
        AreaCenter();
    }

    return std::abs(*area_/2);
}

double ContourImpl::Length() const
{
    if (boost::none == length_)
    {
        double len = 0.;
        for (const auto &vertexes : curves_)
        {
            if (vertexes.size() > 1)
            {
                const int numPoints = static_cast<int>(vertexes.size()) - 1;
                constexpr int simdSize = 8;
                const int regularNumPoints = numPoints & (-simdSize);

                if (is_closed_)
                {
                    len += Util::dist(vertexes[0], vertexes[numPoints]);
                }

                int n = 0;
                const cv::Point2f *pt = vertexes.data();
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
                    len += vcl::horizontal_add(vcl::sqrt(dx*dx + dy * dy));
                    pt += simdSize;
                }

                for (; n < numPoints; ++n, ++pt)
                {
                    len += Util::dist(pt, pt + 1);
                }
            }
        }

        length_ = len;
    }

    return *length_;
}

cv::Point2d ContourImpl::Centroid() const
{
    if (boost::none == centroid_)
    {
        AreaCenter();
    }

    return *centroid_;
}

cv::Rect ContourImpl::BoundingBox() const
{
    if (boost::none == bbox_)
    {
        bbox_ = cv::Rect();
        for (const auto &vertexes : curves_)
        {
            if (!vertexes.empty())
            {
                const int numPoints = static_cast<int>(vertexes.size());
                constexpr int simdSize = 8;
                const int regularNumPoints = numPoints & (-simdSize);

                vcl::Vec8f top(std::numeric_limits<float>::max());
                vcl::Vec8f left(std::numeric_limits<float>::max());
                vcl::Vec8f bot(std::numeric_limits<float>::lowest());
                vcl::Vec8f right(std::numeric_limits<float>::lowest());

                int n = 0;
                const cv::Point2f *pt = vertexes.data();
                for (; n < regularNumPoints; n += simdSize)
                {
                    vcl::Vec8f v1, v2;
                    v1.load(reinterpret_cast<const float *>(pt));
                    v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
                    vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                    vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

                    top = vcl::min(top, y);
                    left = vcl::min(left, x);
                    bot = vcl::max(bot, y);
                    right = vcl::max(right, x);

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

                *bbox_ |= cv::Rect(x, y, w, h);
            }
        }
    }

    return *bbox_;
}

cv::Point3d ContourImpl::SmallestCircle() const
{
    typedef const cv::Point2f* PointIterator;
    typedef const float* CoordIterator;
    typedef Miniball::Miniball<2, Miniball::CoordAccessor<PointIterator, CoordIterator>> MB;

    if (boost::none == mini_ball_)
    {
        mini_ball_ = cv::Point3d();
        if (!Empty())
        {
            if (1 == curves_.size())
            {
                MB mb(curves_.front().data(), curves_.front().data() + curves_.front().size());
                mini_ball_->x = *mb.center();
                mini_ball_->y = *(mb.center()+1);
                mini_ball_->z = std::sqrt(mb.squared_radius());
            }
            else
            {
                std::size_t numPoints = 0;
                for (const auto &c : curves_)
                {
                    numPoints += c.size();
                }

                UScalablePoint2fSequence points(numPoints);
                UScalablePoint2fSequence::pointer pDst = points.data();
                for (const auto &c : curves_)
                {
                    std::memcpy(pDst, c.data(), c.size() * sizeof(ScalablePoint2fSequence::value_type));
                    pDst += c.size();
                }

                MB mb(points.data(), points.data() + points.size());
                mini_ball_->x = *mb.center();
                mini_ball_->y = *(mb.center() + 1);
                mini_ball_->z = std::sqrt(mb.squared_radius());
            }
        }
    }

    return *mini_ball_;
}

double ContourImpl::Circularity() const
{
    if (boost::none == circularity_)
    {
        const double F = ContourImpl::Area();
        const cv::Point2d C = ContourImpl::Centroid();
        if (is_closed_ && K_YES==is_simple_ && F>0. && !curves_.empty())
        {
            const auto &vertexes = curves_.front();
            vcl::Vec8f cx(static_cast<float>(C.x));
            vcl::Vec8f cy(static_cast<float>(C.y));
            vcl::Vec8f vMaxDist(0.f);

            const int numPoints = static_cast<int>(vertexes.size());
            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            int n = 0;
            const cv::Point2f *pt = vertexes.data();
            for (; n < regularNumPoints; n += simdSize)
            {
                vcl::Vec8f v1, v2;
                v1.load(reinterpret_cast<const float *>(pt));
                v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
                vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);
                vMaxDist = vcl::max(vcl::square(x - cx) + vcl::square(y - cy), vMaxDist);

                pt += simdSize;
            }

            double maxDist = vcl::horizontal_max(vMaxDist);
            for (; n < numPoints; ++n, ++pt)
            {
                maxDist = std::max(maxDist, Util::square(pt->x - C.x) + Util::square(pt->y - C.y));
            }

            if (maxDist>0.)
            {
                circularity_ = std::min(1.0, F / (maxDist*CV_PI));
            }
            else
            {
                circularity_ = 0.;
            }
        }
        else
        {
            circularity_ = 0.;
        }
    }

    return *circularity_;
}

cv::Scalar ContourImpl::Diameter() const
{
    if (ContourImpl::Empty())
    {
        return cv::Scalar();
    }
    else
    {
        if (ContourImpl::TestConvex())
        {
            const auto &vertexes = curves_.front();
            const int n = static_cast<int>(vertexes.size());
            return RotatingCaliper::diameter(vertexes.data(), n);
        }
        else
        {
            return ContourImpl::GetConvex()->Diameter();
        }
    }
}

cv::RotatedRect ContourImpl::SmallestRectangle() const
{
    if (ContourImpl::Empty())
    {
        return cv::RotatedRect();
    }
    else
    {
        if (ContourImpl::TestConvex())
        {
            const auto &vertexes = curves_.front();
            const int n = static_cast<int>(vertexes.size());
            return RotatingCaliper::minAreaRect(vertexes.data(), n);
        }
        else
        {
            return ContourImpl::GetConvex()->SmallestRectangle();
        }
    }
}

bool ContourImpl::TestClosed() const
{
    return is_closed_;
}

bool ContourImpl::TestConvex() const
{
    if (K_UNKNOWN != is_convex_)
    {
        return K_YES == is_convex_;
    }
    else
    {
        if (K_YES == is_simple_)
        {
            if (curves_.empty() || curves_.front().empty())
            {
                is_convex_ = K_NO;
                return false;
            }
            else
            {
                if (curves_.front().size()<4)
                {
                    is_convex_ = K_YES;
                    return true;
                }
                else
                {
                    const auto &vertexes = curves_.front();
                    const int n = static_cast<int>(vertexes.size());
                    const cv::Point2f *prev_pt = vertexes.data() + vertexes.size() - 2;
                    const cv::Point2f *cur_pt  = vertexes.data() + vertexes.size() - 1;
                    const cv::Point2f *end_pt  = vertexes.data() + vertexes.size();
                    float dx0 = cur_pt->x - prev_pt->x;
                    float dy0 = cur_pt->y - prev_pt->y;
                    int orientation = 0;

                    for (const cv::Point2f *pt = vertexes.data(); pt != end_pt; ++pt)
                    {
                        prev_pt = cur_pt;
                        cur_pt = pt;

                        const float dx = cur_pt->x - prev_pt->x;
                        const float dy = cur_pt->y - prev_pt->y;
                        const float dxdy0 = dx * dy0;
                        const float dydx0 = dy * dx0;

                        orientation |= (dydx0 > dxdy0) ? 1 : ((dydx0 < dxdy0) ? 2 : 3);
                        if (orientation == 3)
                        {
                            is_convex_ = K_NO;
                            return false;
                        }

                        dx0 = dx;
                        dy0 = dy;
                    }

                    is_convex_ = K_YES;
                    return true;
                }
            }
        }
        else
        {
            return false;
        }
    }
}

bool ContourImpl::TestPoint(const cv::Point2f &point) const
{
    const auto &vertexes = curves_.front();
    if (!is_closed_ || vertexes.size() < 3)
    {
        return false;
    }
    else
    {
        constexpr int simdSize = 8;
        const int numPoints = static_cast<int>(vertexes.size() - 1);
        const int regularNumPoints = numPoints & (-simdSize);

        int n = 0;
        int wn = 0;
        const cv::Point2f *pt = vertexes.data();
        const cv::Point2f *pts = vertexes.data();
        const cv::Point2f *pte = vertexes.data() + vertexes.size() - 1;
        const vcl::Vec8f Px(point.x);
        const vcl::Vec8f Py(point.y);
        for (; n < regularNumPoints; n += simdSize)
        {
            vcl::Vec8f v1, v2;
            v1.load(reinterpret_cast<const float *>(pt));
            v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
            vcl::Vec8f xi = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
            vcl::Vec8f yi = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

            v1.load(reinterpret_cast<const float *>(pt + 1));
            v2.load(reinterpret_cast<const float *>(pt + 1 + simdSize / 2));
            vcl::Vec8f xi1 = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
            vcl::Vec8f yi1 = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);

            vcl::Vec8fb c0 = yi >= Py;
            vcl::Vec8fb c1 = yi1 < Py;
            vcl::Vec8f  l0 = Util::isLeft(xi, yi, xi1, yi1, Px, Py);
            vcl::Vec8fb c2 = l0 > 0;
            vcl::Vec8fb c3 = l0 < 0;
            wn += vcl::horizontal_count(c0 && c1 && c2);
            wn -= vcl::horizontal_count((!c0) && (!c1) && c3);

            pt += simdSize;
        }

        for (; n <= numPoints; ++n, ++pt)
        {
            const cv::Point2f *pt1 = (pt == pte) ? pts : (pt + 1);
            if (pt->y >= point.y) {
                if (pt1->y < point.y)
                {
                    if (Util::isLeft(*pt, *pt1, point) > 0)
                        ++wn;
                }
            }
            else {
                if (pt1->y >= point.y)
                {
                    if (Util::isLeft(*pt, *pt1, point) < 0)
                        --wn;
                }
            }
        }

        return wn & 1;
    }
}

bool ContourImpl::TestSelfIntersection(const cv::String &/*closeContour*/) const
{
    return !is_simple_;
}

}
}
