#ifndef __OPENCV_MVLAB_REGION_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_IMPL_HPP__

#include "contour_impl.hpp"
#include "uvector.h"
#include <opencv2/mvlab/region.hpp>
#include <boost/optional.hpp>
#include <tbb/scalable_allocator.h>
#include <2geom/2geom.h>

namespace cv {
namespace mvlab {

struct RunLength
{
    RunLength(const RunLength &r) : row(r.row), colb(r.colb), cole(r.cole), label(r.label) {}
    RunLength(const int ll, const int bb, const int ee) : row(ll), colb(bb), cole(ee), label(0) {}
    RunLength(const int ll, const int bb, const int ee, const int lab) : row(ll), colb(bb), cole(ee), label(lab) {}

    int row;  // line number (row) of run
    int colb; // column index of beginning(include) of run
    int cole; // column index of ending(exclude) of run
    int label;
};

using RunSequence                   = ao::uvector<RunLength, MyAlloc<RunLength>>;
using RunSequenceSequence           = std::vector<RunSequence, MyAlloc<RunSequence>>;
using RunPtrSequence                = std::vector<RunLength *, MyAlloc<RunLength*>>;
using RunConstPtrSequence           = std::vector<const RunLength *, MyAlloc<const RunLength*>>;
using RowBeginSequence              = ScalableIntSequence;
using RowBeginSequenceSequence      = ScalableIntSequenceSequence;

class RegionImpl : public Region
{
public:
    RegionImpl() { }
    RegionImpl(const Rect2f &rect);
    RegionImpl(const RotatedRect &rotatedRect);
    RegionImpl(const Point2f &center, const float radius);
    RegionImpl(const Point2f &center, const Size2f &size);
    RegionImpl(const Point2f &center, const Size2f &size, const float angle);
    RegionImpl(RunSequence *const runs);

public:
    int Draw(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    Rect BoundingBox() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    int CountRows() const CV_OVERRIDE;
    int GetContour(cv::Ptr<Contour> &contour) const CV_OVERRIDE;
    int GetConvex(cv::Ptr<Contour> &convex) const CV_OVERRIDE;
    int GetPoints(std::vector<cv::Point> &points) const CV_OVERRIDE;
    int GetPolygon(cv::Ptr<Contour> &polygon, const float tolerance) const CV_OVERRIDE;
    int GetRuns(std::vector<cv::Point3i> &runs) const CV_OVERRIDE;

    cv::Ptr<Region> Complement(const cv::Rect &universe) const CV_OVERRIDE;
    cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;

    int Connect(const int connectivity, std::vector<Ptr<Region>> &regions) const CV_OVERRIDE;

public:
    const RunSequence &GetAllRuns() const { return rgn_runs_; }
    const RowBeginSequence &GetRowBeginSequence() const;

private:
    void FromMask(const cv::Mat &mask);
    void FromPathVector(const Geom::PathVector &pv);
    void DrawVerified(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness, const int borderStyle) const;
    void TraceAllContours() const;
    void TraceContour() const;
    void GatherBasicFeatures() const;

private:
    const RunSequence                    rgn_runs_;
    mutable RowBeginSequence             row_begs_;
    mutable boost::optional<double>      area_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Rect>    bbox_;
    mutable std::vector<Ptr<Contour>>    contour_outers_;
    mutable std::vector<Ptr<Contour>>    contour_holes_;
    mutable cv::Ptr<Contour> contour_;
};

}
}

#endif
