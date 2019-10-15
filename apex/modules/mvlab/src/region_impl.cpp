#include "precomp.hpp"
#include "region_impl.hpp"

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

}
}
