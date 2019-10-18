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
    ContourImpl(const Rect2f &rect);
    ContourImpl(const RotatedRect &rotatedRect);
    ContourImpl(const Point2f &center, const float radius);
    ContourImpl(const Point2f &center, const Size2f &size);
    ContourImpl(const Point2f &center, const Size2f &size, const float angle);
    ContourImpl(const Geom::Path &path, const bool closed);

public:
    int Draw(Mat &img, const Scalar& color, float thickness, int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& color, float thickness, int style) const CV_OVERRIDE;

public:
    double Length() const CV_OVERRIDE;
    double Area() const CV_OVERRIDE;
    Point2d Centroid() const CV_OVERRIDE;
    Rect BoundingBox() const CV_OVERRIDE;

public:
    const Geom::Path GetPath() const { return path_; }

private:
    void ClearCacheData();

private:
    bool is_closed_;
    Geom::Path path_;
    mutable boost::optional< Point2f> start_;
    mutable boost::optional<std::vector<Point2f>> points_;
    mutable boost::optional<double> length_;
    mutable boost::optional<double> area_;
    mutable boost::optional<cv::Point2d> centroid_;
    mutable boost::optional<cv::Rect> bbox_;
};

}
}

#endif
