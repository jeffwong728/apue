#ifndef __OPENCV_MVLAB_REGION_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_IMPL_HPP__

#include "contour_impl.hpp"
#include <opencv2/mvlab/region.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <tbb/scalable_allocator.h>
#include <2geom/2geom.h>

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

struct RowRange
{
    RowRange() : begRun(0), endRun(0) {}
    RowRange(const int b, const int e) : begRun(b), endRun(e) {}
    int begRun;
    int endRun;
};

using RunList      = std::vector<RunLength>;
using RowRangeList = std::vector<RowRange>;

class RegionImpl : public Region
{
public:
    RegionImpl() {}
    RegionImpl(const cv::Mat &mask);
    RegionImpl(const Rect2f &rect);
    RegionImpl(const RotatedRect &rotatedRect);
    RegionImpl(const Point2f &center, const float radius);
    RegionImpl(const Point2f &center, const Size2f &size);
    RegionImpl(const Point2f &center, const Size2f &size, const float angle);

public:
    int Draw(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const CV_OVERRIDE;

public:
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    Rect BoundingBox() const CV_OVERRIDE;
    void Connect(std::vector<Ptr<Region>> &regions) const CV_OVERRIDE;

private:
    void ClearCacheData();
    void FromMask(const cv::Mat &mask);
    void FromPathVector(const Geom::PathVector &pv);
    void DrawVerified(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const;
    void TraceContour();
    void TraceContourRunlength();
    void TraceContourMask();

private:
    RunList rgn_runs_;
    cv::Mat rgn_mask_;
    RowRangeList row_ranges_;
    mutable boost::optional<double> area_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Rect>    bbox_;
    mutable boost::optional<std::vector<Ptr<Contour>>> contour_outers_;
    mutable boost::optional<std::vector<Ptr<Contour>>> contour_holes_;
};

}
}

#endif
