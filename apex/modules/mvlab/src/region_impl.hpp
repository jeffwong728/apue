#ifndef __OPENCV_MVLAB_REGION_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_IMPL_HPP__

#include <opencv2/mvlab/region.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <tbb/scalable_allocator.h>

namespace cv {
namespace mvlab {

struct RunLength
{
    RunLength() : row(0), colb(0), cole(0), label(0) {}
    RunLength(const RunLength &r) : row(r.row), colb(r.colb), cole(r.cole), label(r.label) {}
    RunLength(const int ll, const int bb, const int ee) : row(ll), colb(bb), cole(ee), label(0) {}
    RunLength(const int ll, const int bb, const int ee, const int lab) : row(ll), colb(bb), cole(ee), label(lab) {}

    int row;  // line number (row) of run
    int colb; // column index of beginning(include) of run
    int cole; // column index of ending(exclude) of run
    int label;
};

using RunList = std::vector<RunLength>;

class RegionImpl : public Region
{
public:
    RegionImpl() {}
    RegionImpl(const Rect &rect);

public:
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    void Connect(std::vector<Ptr<Region>> &regions) const CV_OVERRIDE;

private:
    RunList data_;
    mutable boost::optional<double> area_;
    mutable boost::optional<cv::Point2d> centroid_;
};

}
}

#endif
