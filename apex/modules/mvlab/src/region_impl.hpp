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
    RegionImpl(RunSequence *const runs);

public:
    int Draw(Mat &img, const Scalar& fillColor) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& fillColor) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    Rect BoundingBox() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    int CountRows() const CV_OVERRIDE;
    // Access
    cv::Ptr<Contour> GetContour() const CV_OVERRIDE;
    cv::Ptr<Contour> GetConvex() const CV_OVERRIDE;
    cv::Ptr<Contour> GetPolygon(const float tolerance) const CV_OVERRIDE;
    int GetPoints(std::vector<cv::Point> &points) const CV_OVERRIDE;
    int GetRuns(std::vector<cv::Point3i> &runs) const CV_OVERRIDE;
    // Sets
    cv::Ptr<Region> Complement(const cv::Rect &universe) const CV_OVERRIDE;
    cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    cv::Ptr<Region> Union1(const std::vector<cv::Ptr<Region>> &otherRgns) const CV_OVERRIDE;
    cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    // Tests
    bool TestEqual(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    bool TestPoint(const cv::Point &point) const CV_OVERRIDE;
    bool TestSubset(const cv::Ptr<Region> &otherRgn) const CV_OVERRIDE;
    // Geometric Transformations
    cv::Ptr<Region> Move(const cv::Point &delta) const CV_OVERRIDE;
    cv::Ptr<Region> Zoom(const cv::Size2f &scale) const CV_OVERRIDE;
    int Connect(std::vector<Ptr<Region>> &regions) const CV_OVERRIDE;

public:
    const RunSequence &GetAllRuns() const { return rgn_runs_; }
    const RowBeginSequence &GetRowBeginSequence() const;

private:
    void FromMask(const cv::Mat &mask);
    void FromPathVector(const Geom::PathVector &pv);
    void DrawVerified(Mat &img, const Scalar& fillColor) const;
    void DrawVerifiedGray(Mat &img, const Scalar& fillColor) const;
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
