#ifndef __OPENCV_MVLAB_REGION_HPP__
#define __OPENCV_MVLAB_REGION_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Region {

public:
    Region() {}
    virtual ~Region() {}

    CV_WRAP static cv::Ptr<Region> CreateEmpty();
    CV_WRAP static cv::Ptr<Region> CreateRectangle(const cv::Rect2f &rect);
    CV_WRAP static cv::Ptr<Region> CreateRotatedRectangle(const cv::RotatedRect &rotatedRect);
    CV_WRAP static cv::Ptr<Region> CreateCircle(const cv::Point2f &center, const float radius);
    CV_WRAP static cv::Ptr<Region> CreateEllipse(const cv::Point2f &center, const cv::Size2f &size);
    CV_WRAP static cv::Ptr<Region> CreateRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float angle);
    CV_WRAP static cv::Ptr<Region> CreatePolygon(const std::vector<cv::Point2f> &vertexes);

public:
    virtual int Draw(Mat &img, const Scalar& fillColor, const Scalar& borderColor, const float borderThickness = 1, const int borderStyle = 0) const = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual cv::Point2d Centroid() const = 0;
    CV_WRAP virtual cv::Rect BoundingBox() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual int CountRows() const = 0;
    CV_WRAP virtual int GetContour(CV_OUT cv::Ptr<Contour> &contour) const = 0;
    CV_WRAP virtual int GetConvex(CV_OUT cv::Ptr<Contour> &convex) const = 0;
    CV_WRAP virtual int GetPoints(CV_OUT std::vector<cv::Point> &points) const = 0;
    CV_WRAP virtual int GetPolygon(CV_OUT cv::Ptr<Contour> &polygon, const float tolerance) const = 0;
    CV_WRAP virtual int GetRuns(CV_OUT std::vector<cv::Point3i> &runs) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Complement(const cv::Rect &universe) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union1(const std::vector<cv::Ptr<Region>> &otherRgns) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestEqual(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestPoint(const cv::Point &point) const = 0;
    CV_WRAP virtual bool TestSubset(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual int Connect(const int connectivity, CV_OUT std::vector<cv::Ptr<Region>> &regions) const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Scalar& fillColor, const cv::Scalar& borderColor, const float borderThickness = 1, const int borderStyle = 0) const = 0;
};

}
}

#endif
