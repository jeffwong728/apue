#ifndef __OPENCV_MVLAB_CONTOUR_IMPL_HPP__
#define __OPENCV_MVLAB_CONTOUR_IMPL_HPP__

#include <opencv2/mvlab/contour.hpp>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <tbb/scalable_allocator.h>
#include <2geom/2geom.h>

namespace cv {
namespace mvlab {

class ContourImpl : public Contour
{
public:
    ContourImpl() : is_closed_(false) {}
    ContourImpl(const std::vector<Point2f> &vertexes, const bool closed);
    ContourImpl(Point2fSequence *vertexes, const bool closed);

public:
    int Draw(Mat &img, const Scalar& color, const float thickness, const int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& color, float thickness, const int style) const CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    double Length() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    cv::Point2d Centroid() const CV_OVERRIDE;
    cv::Rect BoundingBox() const CV_OVERRIDE;
    Ptr<Contour> Simplify(const float tolerance) const CV_OVERRIDE;
    //Access
    int GetPoints(std::vector<Point2f> &vertexes) const CV_OVERRIDE;
    //Geometric Transformations
    cv::Ptr<Contour> Move(const cv::Point2f &delta) const CV_OVERRIDE;
    cv::Ptr<Contour> Zoom(const cv::Size2f &scale) const CV_OVERRIDE;
    //Features
    bool TestClosed() const CV_OVERRIDE;
    bool TestPoint(const cv::Point2f &point) const CV_OVERRIDE;
    bool TestSelfIntersection() const CV_OVERRIDE;

public:
    void Feed(Cairo::RefPtr<Cairo::Context> &cr) const;

public:
    const Point2fSequence &GetVertexes() const { return vertexes_; }

private:
    void DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const;
    void AreaCenter() const;
    void ChangedCoordinatesToFixed() const;

private:
    const bool is_closed_;
    const Point2fSequence vertexes_;
    mutable UScalableIntSequence x_fixed_;
    mutable UScalableIntSequence y_fixed_;
    mutable boost::optional<double> length_;
    mutable boost::optional<double> area_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Rect> bbox_;
};

}
}

#endif
