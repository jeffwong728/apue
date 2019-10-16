#include "precomp.hpp"
#include "region_impl.hpp"
#include "utility.hpp"
#include <2geom/circle.h>
#include <2geom/ellipse.h>
#include <2geom/path-sink.h>

namespace cv {
namespace mvlab {

RegionImpl::RegionImpl(const Rect &rect)
    : Region()
{
    if (rect.width > 0 && rect.height > 0)
    {
        data_.reserve(rect.height);
        for (int y = 0; y < rect.height; ++y)
        {
            data_.emplace_back(y, rect.x, rect.x + rect.width);
        }
    }
}

RegionImpl::RegionImpl(const RotatedRect &rotatedRect)
    : Region()
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

    FromPathVector(pv);
}

RegionImpl::RegionImpl(const Point2f &center, const float radius)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Circle(center.x, center.y, radius)));
    FromPathVector(pv);
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, 0.0)));
    FromPathVector(pv);
}

RegionImpl::RegionImpl(const Point2f &center, const Size2f &size, const float angle)
    : Region()
{
    Geom::PathVector pv(Geom::Path(Geom::Ellipse(center.x, center.y, size.width, size.height, angle)));
    FromPathVector(pv);
}

double RegionImpl::Area() const
{
    if (area_ == boost::none)
    {
        double a = 0;
        double x = 0;
        double y = 0;

        for (const RunLength &rl : data_)
        {
            const auto n = rl.cole - rl.colb;
            a += n;
            x += (rl.cole - 1 + rl.colb) * n / 2.0;
            y += rl.row * n;
        }

        area_ = a;
        if (a > 0)
        {
            centroid_.emplace(x / a, y / a);
        }
        else
        {
            centroid_.emplace(0, 0);
        }
    }

    return *area_;
}

cv::Point2d RegionImpl::Centroid() const
{
    if (centroid_ == boost::none)
    {
        Area();
    }

    return *centroid_;
}

void RegionImpl::Connect(std::vector<Ptr<Region>> &regions) const
{
    regions.resize(0);
    regions.push_back(makePtr<RegionImpl>());
    regions.push_back(makePtr<RegionImpl>());
    regions.push_back(makePtr<RegionImpl>());
}

void RegionImpl::ClearCacheData()
{
    area_ = boost::none;
}

void RegionImpl::FromMask(const cv::Mat &mask)
{
    int dph = mask.depth();
    int cnl = mask.channels();
    if (CV_8U == dph && 1 == cnl)
    {
        constexpr int top = 0;
        const int bot = mask.rows;
        constexpr int left = 0;
        const int right = mask.cols;
        for (int r = top; r < bot; ++r)
        {
            int cb = -1;
            const uchar* pRow = mask.data + r * mask.step1();
            for (int c = left; c < right; ++c)
            {
                if (pRow[c])
                {
                    if (cb < 0)
                    {
                        cb = c;
                    }
                }
                else
                {
                    if (cb > -1)
                    {
                        data_.emplace_back(r, cb, c);
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                data_.emplace_back(r, cb, right);
            }
        }
    }

    ClearCacheData();
}

void RegionImpl::FromPathVector(const Geom::PathVector &pv)
{
    std::vector<uint8_t> buf;
    Geom::OptRect bbox = pv.boundsFast();
    if (bbox)
    {
        int t = cvFloor(bbox.get().top());
        int b = cvCeil(bbox.get().bottom()) + 1;
        int l = cvFloor(bbox.get().left());
        int r = cvCeil(bbox.get().right()) + 1;
        cv::Rect rect(cv::Point(l - 3, t - 3), cv::Point(r + 3, b + 3));
        cv::Mat mask = Util::PathToMask(pv*Geom::Translate(-rect.x, -rect.y), rect.size(), buf);
        FromMask(mask);

        for (RunLength &run : data_)
        {
            run.colb += rect.x;
            run.cole += rect.x;
            run.row += rect.y;
        }
    }
}

}
}
