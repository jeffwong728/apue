#include "precomp.hpp"
#include "contour_impl.hpp"
#include "utility.hpp"
#include "convex_hull.hpp"
#include "rot_calipers.hpp"
#include "Miniball.hpp"
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(cv::mvlab::Contour)
BOOST_CLASS_EXPORT_IMPLEMENT(cv::mvlab::ContourImpl)

namespace cv {
namespace mvlab {
const cv::String ContourImpl::type_guid_s{ "55F24E48-3E9E-4164-80E0-DAB56F7735B2" };

ContourImpl::ContourImpl(const std::vector<Point2f> &vertexes, const int is_self_intersect)
    : Contour()
    , is_self_intersect_(is_self_intersect)
    , is_convex_(K_UNKNOWN)
    , curves_(1, ScalablePoint2fSequence(vertexes.cbegin(), vertexes.cend()))
{
}

ContourImpl::ContourImpl(ScalablePoint2fSequence *vertexes, const int is_self_intersect)
    : Contour()
    , is_self_intersect_(is_self_intersect)
    , is_convex_(K_UNKNOWN)
    , curves_(1)
{
    vertexes->swap(const_cast<ScalablePoint2fSequence &>(curves_.front()));
}

ContourImpl::ContourImpl(ScalablePoint2fSequenceSequence *curves, const int is_self_intersect)
    : Contour()
    , is_self_intersect_(is_self_intersect)
    , is_convex_(K_UNKNOWN)
    , curves_(std::move(*curves))
{
}

ContourImpl::ContourImpl(const std::string &bytes)
{
    try
    {
        std::istringstream iss(bytes, std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_istream fin;
        fin.push(boost::iostreams::lzma_decompressor());
        fin.push(iss);

        boost::archive::text_iarchive ia(fin);
        ia >> boost::serialization::make_nvp("contour", *this);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
    }
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
    return 1;
}

int ContourImpl::CountCurves() const
{
    return static_cast<int>(curves_.size());
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

void ContourImpl::GetSmallestRectangle(std::vector< cv::RotatedRect> &miniRects) const
{
    miniRects.resize(0);
    miniRects.push_back(ContourImpl::SmallestRectangle());
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
            cv::approxPolyDP(Point2fSequence(c.cbegin(), c.cend()), approxCurve, tolerance, IsCurveClosed(c));
            if (approxCurve.size()>1 && IsCurveClosed(c))
            {
                const cv::Point2f dxy = approxCurve.front() - approxCurve.back();
                if (dxy.dot(dxy) > 0.f)
                {
                    ScalablePoint2fSequence tPoints(approxCurve.size()+1);
                    std::memcpy(tPoints.data(), approxCurve.data(), sizeof(cv::Point2f)*approxCurve.size());
                    tPoints.back() = tPoints.front();
                    itCurve->swap(tPoints);
                }
                else
                {
                    ScalablePoint2fSequence tPoints(approxCurve.cbegin(), approxCurve.cend());
                    itCurve->swap(tPoints);
                }
            }
            else
            {
                ScalablePoint2fSequence tPoints(approxCurve.cbegin(), approxCurve.cend());
                itCurve->swap(tPoints);
            }

            ++itCurve;
        }

        return makePtr<ContourImpl>(&approxCurves, is_self_intersect_);
    }
}

cv::Ptr<Contour> ContourImpl::GetConvex() const
{
    if (convex_hull_)
    {
        return convex_hull_;
    }

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
                contImp->is_self_intersect_ = K_NO;
                contImp->is_convex_ = K_YES;
            }
            return convex_hull_;
        }
    }

    return makePtr<ContourImpl>();
}

void ContourImpl::GetPoints(std::vector<cv::Point2f> &points) const
{
    if (curves_.empty())
    {
        points.clear();
    }
    else
    {
        points.resize(ContourImpl::CountPoints());
        cv::Point2f *dst = points.data();
        for (const auto &crv : curves_)
        {
            const int cPoints = static_cast<int>(crv.size()) - IsCurveClosed(crv);
            std::memcpy(dst, crv.data(), sizeof(cv::Point2f)*cPoints);
            dst += cPoints;
        }
    }
}

void ContourImpl::SelectPoints(const int index, std::vector<cv::Point2f> &points) const
{
    points.resize(0);
    if (!curves_.empty() && index>=0 && index<ContourImpl::CountCurves() && !curves_[index].empty())
    {
        const int cPoints = static_cast<int>(curves_[index].size()) - IsCurveClosed(curves_[index]);
        points.resize(cPoints);
        cv::Point2f *dst = points.data();
        std::memcpy(dst, curves_[index].data(), sizeof(cv::Point2f)*cPoints);
    }
}

cv::Ptr<Contour> ContourImpl::SelectObj(const int index) const
{
    if (0 == index)
    {
        return (cv::Ptr<ContourImpl>)const_cast<ContourImpl *>(this)->shared_from_this();
    }
    else
    {
        return cv::Ptr<ContourImpl>();
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

    return makePtr<ContourImpl>(&curves, is_self_intersect_);
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

    return makePtr<ContourImpl>(&curves, is_self_intersect_);
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

    return makePtr<ContourImpl>(&curves, is_self_intersect_);
}

void ContourImpl::GetTestClosed(std::vector<int> &isClosed) const
{
    isClosed.resize(0);
    isClosed.reserve(ContourImpl::CountCurves());
    for (const auto &crv : curves_)
    {
        isClosed.push_back(IsCurveClosed(crv));
    }
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

void ContourImpl::GetTestSelfIntersection(const cv::String &closeContour, std::vector<int> &doesIntersect) const
{
    doesIntersect.resize(0);
    doesIntersect.push_back(ContourImpl::TestSelfIntersection(closeContour));
}

void ContourImpl::Feed(Cairo::RefPtr<Cairo::Context> &cr) const
{
    for (const auto &crv : curves_)
    {
        const int numVertexes = static_cast<int>(crv.size()) - IsCurveClosed(crv);
        if (numVertexes > 1)
        {
            cr->move_to(crv.front().x, crv.front().y);
            for (int i = 1; i < numVertexes; ++i)
            {
                cr->line_to(crv[i].x, crv[i].y);
            }

            if (IsCurveClosed(crv))
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
    if (ContourImpl::TestClosed())
    {
        double area = 0;
        double cx = 0, cy = 0;
        double c20 = 0, c11 = 0, c02 = 0;
        for (const auto &crv : curves_)
        {
            const int numPoints = static_cast<int>(crv.size()) - 1;
            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            int n = 0;
            const cv::Point2f *pt = crv.data();
            vcl::Vec8f va(0.f), vcx(0.f), vcy(0.f), v20(0.f), v11(0.f), v02(0.f);
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
                va += a;
                vcx += (x + xprev)*a;
                vcy += (y + yprev)*a;
                v20 += (x*x + x*xprev + xprev*xprev) * a;
                v11 += (2*x*y + x*yprev + xprev*y + 2*xprev*yprev) * a;
                v02 += (y*y + y * yprev + yprev * yprev) * a;
                pt += simdSize;
            }

            area += vcl::horizontal_add(va);
            cx += vcl::horizontal_add(vcx);
            cy += vcl::horizontal_add(vcy);
            c20 += vcl::horizontal_add(v20);
            c11 += vcl::horizontal_add(v11);
            c02 += vcl::horizontal_add(v02);

            for (; n < numPoints; ++n)
            {
                const int n1 = n + 1;
                const float xprev = crv[n].x;
                const float x = crv[n1].x;
                const float yprev = crv[n].y;
                const float y = crv[n1].y;
                const double a = xprev * y - x * yprev;
                area += a;
                cx += (x + xprev)*a;
                cy += (y + yprev)*a;
                c20 += (x*x + x * xprev + xprev * xprev) * a;
                c11 += (2 * x*y + x * yprev + xprev * y + 2 * xprev*yprev) * a;
                c02 += (y*y + y * yprev + yprev * yprev) * a;
            }
        }

        area_ = area;
        if (std::abs(area) > G_D_TOL)
        {
            centroid_ = cv::Point2d(cx / (3 * area), cy / (3 * area));
            moment_2rd_ = cv::Point3d(c20/(6*area), c11 / (12 * area), c02 / (6 * area));
        }
        else
        {
            centroid_ = ContourImpl::PointsCenter();
            moment_2rd_ = ContourImpl::PointsMoments();
        }
    }
    else
    {
        area_ = 0.;
        centroid_ = ContourImpl::PointsCenter();
        moment_2rd_ = ContourImpl::PointsMoments();
    }
}

void ContourImpl::PointsCloudMoments() const
{
    double sx = 0.;
    double sy = 0.;
    double sxx = 0.;
    double sxy = 0.;
    double syy = 0.;
    int numTotalPoints = 0;
    for (const auto &crv : curves_)
    {
        if (!crv.empty())
        {
            const cv::Point2f dxy = crv.front() - crv.back();
            const int numPoints = static_cast<int>(crv.size()) - (dxy.dot(dxy) < std::numeric_limits<float>::epsilon());
            numTotalPoints += numPoints;

            constexpr int simdSize = 8;
            const int regularNumPoints = numPoints & (-simdSize);

            int n = 0;
            const cv::Point2f *pt = crv.data();
            vcl::Vec8f svx(0), svy(0), svxx(0), svxy(0), svyy(0);
            for (; n < regularNumPoints; n += simdSize)
            {
                vcl::Vec8f v1, v2;
                v1.load(reinterpret_cast<const float *>(pt));
                v2.load(reinterpret_cast<const float *>(pt + simdSize / 2));
                vcl::Vec8f x = vcl::blend8<0, 2, 4, 6, 8, 10, 12, 14>(v1, v2);
                vcl::Vec8f y = vcl::blend8<1, 3, 5, 7, 9, 11, 13, 15>(v1, v2);
                svx += x;
                svy += y;
                svxx += x * x;
                svxy += x * y;
                svyy += y * y;

                pt += simdSize;
            }

            sx += vcl::horizontal_add(svx);
            sy += vcl::horizontal_add(svy);
            sxx += vcl::horizontal_add(svxx);
            sxy += vcl::horizontal_add(svxy);
            syy += vcl::horizontal_add(svyy);

            for (; n < numPoints; ++n, ++pt)
            {
                sx += pt->x;
                sy += pt->y;
                sxx += pt->x * pt->x;
                sxy += pt->x * pt->y;
                syy += pt->y * pt->y;
            }
        }
    }

    if (numTotalPoints)
    {
        point_cloud_center_ = cv::Point2d(sx / numTotalPoints, sy / numTotalPoints);
        point_cloud_moment_2rd_ = cv::Point3d(sxx / numTotalPoints, sxy / numTotalPoints, syy / numTotalPoints);
    }
    else
    {
        point_cloud_center_ = cv::Point2d();
        point_cloud_moment_2rd_ = cv::Point3d();
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
            ScalablePoint2fSequence tPoints = ConvexHull::MelkmanSimpleHull(curves_.front().data(), static_cast<int>(curves_.front().size())-1);
            return cv::makePtr<ContourImpl>(&tPoints, K_NO);
        }
        else if ("Sklansky" == optVal)
        {
            ScalablePoint2fSequence tPoints = ConvexHull::Sklansky(curves_.front().data(), static_cast<int>(curves_.front().size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_NO);
        }
        else
        {
            ScalablePoint2fSequence tPoints = ConvexHull::AndrewMonotoneChain(curves_.front().data(), static_cast<int>(curves_.front().size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_NO);
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
            return cv::makePtr<ContourImpl>(&tPoints, K_NO);
        }
        else
        {
            ScalablePoint2fSequence tPoints = ConvexHull::AndrewMonotoneChain(points.data(), static_cast<int>(points.size()));
            return cv::makePtr<ContourImpl>(&tPoints, K_NO);
        }
    }
}

bool ContourImpl::IsCurveClosed(const ScalablePoint2fSequence &crv)
{
    if (crv.size() < 2)
    {
        return false;
    }
    else
    {
        const cv::Point2f dxy = crv.front() - crv.back();
        return dxy.dot(dxy) < std::numeric_limits<float>::epsilon();
    }
}

int ContourImpl::CountPoints() const
{
    int cPoints = 0;
    for (const auto &crv : curves_)
    {
        cPoints += static_cast<int>(crv.size()) - IsCurveClosed(crv);
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
        for (const auto &crv : curves_)
        {
            if (crv.size() > 1)
            {
                const int numPoints = static_cast<int>(crv.size()) - 1;
                constexpr int simdSize = 8;
                const int regularNumPoints = numPoints & (-simdSize);

                int n = 0;
                const cv::Point2f *pt = crv.data();
                vcl::Vec8f vlen(0.f);
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
                    vlen += vcl::sqrt(dx * dx + dy * dy);
                    pt += simdSize;
                }

                len += vcl::horizontal_add(vlen);

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

cv::Point2d ContourImpl::PointsCenter() const
{
    if (boost::none == point_cloud_center_)
    {
        PointsCloudMoments();
    }

    return *point_cloud_center_;
}

cv::Point3d ContourImpl::Moments() const
{
    if (boost::none == moment_2rd_)
    {
        AreaCenter();
    }

    return *moment_2rd_;
}

cv::Point3d ContourImpl::PointsMoments() const
{
    if (boost::none == point_cloud_moment_2rd_)
    {
        PointsCloudMoments();
    }

    return *point_cloud_moment_2rd_;
}

cv::Rect ContourImpl::BoundingBox() const
{
    if (boost::none == bbox_)
    {
        bbox_ = cv::Rect();
        for (const auto &crv : curves_)
        {
            if (!crv.empty())
            {
                const int numPoints = static_cast<int>(crv.size());
                constexpr int simdSize = 8;
                const int regularNumPoints = numPoints & (-simdSize);

                vcl::Vec8f top(std::numeric_limits<float>::max());
                vcl::Vec8f left(std::numeric_limits<float>::max());
                vcl::Vec8f bot(std::numeric_limits<float>::lowest());
                vcl::Vec8f right(std::numeric_limits<float>::lowest());

                int n = 0;
                const cv::Point2f *pt = crv.data();
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
        if (ContourImpl::TestClosed() && F>0. && !curves_.empty())
        {
            double maxDist = std::numeric_limits<double>::lowest();
            for (const auto &crv : curves_)
            {
                vcl::Vec8f cx(static_cast<float>(C.x));
                vcl::Vec8f cy(static_cast<float>(C.y));
                vcl::Vec8f vMaxDist(0.f);

                const int numPoints = static_cast<int>(crv.size());
                constexpr int simdSize = 8;
                const int regularNumPoints = numPoints & (-simdSize);

                int n = 0;
                const cv::Point2f *pt = crv.data();
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

                maxDist = std::max<double>(vcl::horizontal_max(vMaxDist), maxDist);
                for (; n < numPoints; ++n, ++pt)
                {
                    maxDist = std::max(maxDist, Util::square(pt->x - C.x) + Util::square(pt->y - C.y));
                }
            }

            if (maxDist > G_D_TOL)
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

double ContourImpl::Convexity() const
{
    if (boost::none == convexity_)
    {
        const double Fc = ContourImpl::GetConvex()->Area();
        if (Fc > 0.)
        {
            const double Fo = ContourImpl::Area();
            convexity_ = std::min(1., Fo / Fc);
        }
        else
        {
            convexity_ = 0.;
        }
    }

    return *convexity_;
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
            const int n = static_cast<int>(vertexes.size())-ContourImpl::TestClosed();
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
            const int n = static_cast<int>(vertexes.size()) - ContourImpl::TestClosed();
            return RotatingCaliper::minAreaRect(vertexes.data(), n);
        }
        else
        {
            return ContourImpl::GetConvex()->SmallestRectangle();
        }
    }
}

cv::Point3d ContourImpl::EllipticAxis() const
{
    cv::Point2d c = ContourImpl::Centroid();
    cv::Point3d m = ContourImpl::Moments();
    const double m02 = m.x - c.x * c.x;
    const double m11 = m.y - c.x * c.y;
    const double m20 = m.z - c.y * c.y;

    const double mt = std::sqrt(Util::square(m20-m02)+4*Util::square(m11));
    const double Ra = std::sqrt(8 * std::max(0., m20 + m02 + mt)) / 2;
    const double Rb = std::sqrt(8 * std::max(0., m20 + m02 - mt)) / 2;


    const double x = m02 - m20;
    if (x)
    {
        const bool rsym = std::abs(2 * m11 / (m02 + m20)) < 1e-3 && std::abs(m11 / x) > 1e-3;
        if (rsym)
        {
            return cv::Point3d(Ra, Rb, 0.);
        }
        else
        {
            const double Phi = Util::deg(0.5 * std::atan2(2 * m11, x));
            return cv::Point3d(Ra, Rb, Phi);
        }
    }
    else
    {
        return cv::Point3d(Ra, Rb, 0.);
    }
}

double ContourImpl::Anisometry() const
{
    cv::Point3d e = ContourImpl::EllipticAxis();
    const double Ra = e.x;
    const double Rb = e.y;
    if (Rb)
    {
        return Ra / Rb;
    }
    else
    {
        return 0.;
    }
}

double ContourImpl::Bulkiness() const
{
    cv::Point3d e = ContourImpl::EllipticAxis();
    const double Ra = e.x;
    const double Rb = e.y;
    const double A = ContourImpl::Area();
    if (A)
    {
        return CV_PI * Ra * Rb / A;
    }
    else
    {
        return 0.;
    }
}

double ContourImpl::StructureFactor() const
{
    return ContourImpl::Anisometry() * ContourImpl::Bulkiness() - 1.0;
}

bool ContourImpl::TestClosed() const
{
    if (curves_.empty())
    {
        return false;
    }
    else
    {
        for (const auto &crv : curves_)
        {
            if (!IsCurveClosed(crv))
            {
                return false;
            }
        }
        return true;
    }
}

bool ContourImpl::TestConvex() const
{
    if (K_UNKNOWN != is_convex_)
    {
        return K_YES == is_convex_;
    }

    if ((1 != curves_.size()) || curves_.front().empty() || !ContourImpl::TestClosed() || ContourImpl::TestSelfIntersection("true"))
    {
        is_convex_ = K_NO;
        return false;
    }

    if (curves_.front().size() < 4)
    {
        is_convex_ = K_YES;
        return true;
    }

    const auto &vertexes = curves_.front();
    const int n = static_cast<int>(vertexes.size()) - ContourImpl::TestClosed();
    const cv::Point2f *prev_pt = vertexes.data() + n - 2;
    const cv::Point2f *cur_pt = vertexes.data() + n - 1;
    const cv::Point2f *end_pt = vertexes.data() + n;
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

bool ContourImpl::TestPoint(const cv::Point2f &point) const
{
    if (!ContourImpl::TestClosed())
    {
        return false;
    }
    else
    {
        int wn = 0;
        for (const auto &crv : curves_)
        {
            if (crv.size() < 4)
            {
                continue;
            }

            constexpr int simdSize = 8;
            const int numPoints = static_cast<int>(crv.size() - 1);
            const int regularNumPoints = numPoints & (-simdSize);

            int n = 0;
            const cv::Point2f *pt = crv.data();
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

            for (; n < numPoints; ++n, ++pt)
            {
                const cv::Point2f *pt1 = pt + 1;
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
        }

        return wn & 1;
    }
}

bool ContourImpl::TestSelfIntersection(const cv::String &/*closeContour*/) const
{
    if (K_UNKNOWN == is_self_intersect_)
    {
        is_self_intersect_ = K_NO;
        for (const auto &crv : curves_)
        {
            if (boost::geometry::intersects(crv))
            {
                is_self_intersect_ = K_YES;
                break;
            }
        }
    }

    return K_YES == is_self_intersect_;
}

FitLineResults ContourImpl::FitLine(const FitLineParameters &/*parameters*/) const
{
    return FitLineResults();
}

cv::String ContourImpl::GetErrorStatus() const
{
    return err_msg_;
}

int ContourImpl::Save(const cv::String &fileName, const cv::Ptr<Dict> &opts) const
{
    return WriteToFile<ContourImpl>(*this, "contour", fileName, opts, err_msg_);
}

int ContourImpl::Load(const cv::String &fileName, const cv::Ptr<Dict> &opts)
{
    return LoadFromFile<ContourImpl>(*this, "contour", fileName, opts, err_msg_);
}

int ContourImpl::Serialize(const cv::String &name, H5Group *g) const
{
    err_msg_.resize(0);
    std::ostringstream bytes;
    H5GroupImpl *group = dynamic_cast<H5GroupImpl *>(g);
    if (!group || !group->Valid())
    {
        err_msg_ = "invalid database";
        return MLR_H5DB_INVALID;
    }

    try
    {
        {
            boost::iostreams::filtering_ostream fout;
            fout.push(boost::iostreams::lzma_compressor());
            fout.push(bytes);
            boost::archive::text_oarchive oa(fout);
            oa << boost::serialization::make_nvp("contour", *this);
        }

        H5::DataSet dataSet;
        int r = group->SetDataSet(name, bytes.str(), dataSet);
        if (MLR_SUCCESS != r)
        {
            err_msg_ = "save database failed";
            return r;
        }

        group->SetAttribute(dataSet, cv::String("TypeGUID"),        ContourImpl::TypeGUID());
        group->SetAttribute(dataSet, cv::String("Version"),         0);
        group->SetAttribute(dataSet, cv::String("IsSelfIntersect"), is_self_intersect_);
        group->SetAttribute(dataSet, cv::String("IsConvex"),        is_convex_);
    }
    catch (const std::exception &e)
    {
        err_msg_ = e.what();
        return MLR_IO_STREAM_EXCEPTION;
    }

    return MLR_SUCCESS;
}

}
}