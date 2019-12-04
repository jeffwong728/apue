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

    CV_WRAP static cv::Ptr<Region> GenEmpty();
    CV_WRAP static cv::Ptr<Region> GenChecker(const cv::Size &sizeRegion, const cv::Size &sizePattern);
    CV_WRAP static cv::Ptr<Region> GenTriangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3);
    CV_WRAP static cv::Ptr<Region> GenQuadrangle(const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3, const cv::Point2f &v4);
    CV_WRAP static cv::Ptr<Region> GenRectangle(const cv::Rect2f &rect);
    CV_WRAP static cv::Ptr<Region> GenRotatedRectangle(const cv::RotatedRect &rotatedRect);
    CV_WRAP static cv::Ptr<Region> GenCircle(const cv::Point2f &center, const float radius);
    CV_WRAP static cv::Ptr<Region> GenCircleSector(const cv::Point2f &center, const float radius, const float startAngle, const float endAngle);
    CV_WRAP static cv::Ptr<Region> GenEllipse(const cv::Point2f &center, const cv::Size2f &size);
    CV_WRAP static cv::Ptr<Region> GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float phi);
    CV_WRAP static cv::Ptr<Region> GenEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float phi, const float startAngle, const float endAngle);
    CV_WRAP static cv::Ptr<Region> GenPolygon(const std::vector<cv::Point2f> &vertexes);

public:
    virtual int Draw(Mat &img, const Scalar& fillColor) const = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual int CountRows() const = 0;
    CV_WRAP virtual int CountConnect() const = 0;
    CV_WRAP virtual int CountHoles() const = 0;
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual cv::Point2d Centroid() const = 0;
    CV_WRAP virtual cv::Rect BoundingBox() const = 0;
    CV_WRAP virtual double AreaHoles() const = 0;
    CV_WRAP virtual double Contlength() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetContour() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetHole() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetConvex() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetPolygon(const float tolerance) const = 0;
    CV_WRAP virtual int GetPoints(CV_OUT std::vector<cv::Point> &points) const = 0;
    CV_WRAP virtual int GetRuns(CV_OUT std::vector<cv::Point3i> &runs) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Complement(const cv::Rect &universe) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Difference(const cv::Ptr<Region> &subRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Intersection(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> SymmDifference(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union1() const = 0;
    CV_WRAP virtual cv::Ptr<Region> Union2(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestEqual(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual bool TestPoint(const cv::Point &point) const = 0;
    CV_WRAP virtual bool TestSubset(const cv::Ptr<Region> &otherRgn) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Move(const cv::Point &delta) const = 0;
    CV_WRAP virtual cv::Ptr<Region> Zoom(const cv::Size2f &scale) const = 0;
    CV_WRAP virtual cv::Ptr<Region> AffineTrans(const cv::Matx33d &homoMat2D) const = 0;
    CV_WRAP virtual int Connect(CV_OUT std::vector<cv::Ptr<Region>> &regions) const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Scalar& fillColor) const = 0;
};

}
}

#endif
