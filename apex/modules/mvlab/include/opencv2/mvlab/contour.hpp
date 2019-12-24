#ifndef __OPENCV_MVLAB_CONTOUR_HPP__
#define __OPENCV_MVLAB_CONTOUR_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Contour {

public:
    Contour() {}
    virtual ~Contour() {}

    CV_WRAP static cv::Ptr<Contour> GenEmpty();
    CV_WRAP static cv::Ptr<Contour> GenRectangle(const cv::Rect2f &rect);
    CV_WRAP static cv::Ptr<Contour> GenRotatedRectangle(const cv::RotatedRect &rotatedRect);
    CV_WRAP static cv::Ptr<Contour> GenCircle(const cv::Point2f &center, const float radius, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenCircleSector(const cv::Point2f &center, const float radius, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenEllipse(const cv::Point2f &center, const cv::Size2f &size, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float angle, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenRotatedEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float angle, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static cv::Ptr<Contour> GenPolygon(const std::vector<cv::Point2f> &vertexes);
    CV_WRAP static cv::Ptr<Contour> GenPolyline(const std::vector<cv::Point2f> &vertexes);
    CV_WRAP static cv::Ptr<Contour> GenPolygonRounded(const std::vector<cv::Point2f> &vertexes, const std::vector<cv::Size2f> &radius, const float samplingInterval);
    CV_WRAP static cv::Ptr<Contour> GenCross(const std::vector<cv::Point2f> &center, const std::vector<cv::Size2f> &size, const std::vector<float> &angle);
    CV_WRAP static cv::Ptr<Contour> ReadWkt(const cv::String &wkt);

public:
    virtual int Draw(cv::Mat &img, const cv::Scalar& color, const float thickness = 1, const int style = 0) const = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual int CountPoints() const = 0;
    CV_WRAP virtual void GetCountPoints(CV_OUT std::vector<int> &cPoints) const = 0;
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual double Length() const = 0;
    CV_WRAP virtual cv::Point2d Centroid() const = 0;
    CV_WRAP virtual cv::Point2d PointsCenter() const = 0;
    CV_WRAP virtual cv::Rect BoundingBox() const = 0;
    CV_WRAP virtual cv::Point3d SmallestCircle() const = 0;
    CV_WRAP virtual double Circularity() const = 0;
    CV_WRAP virtual cv::Scalar Diameter() const = 0;
    CV_WRAP virtual cv::RotatedRect SmallestRectangle() const = 0;
    CV_WRAP virtual void GetArea(CV_OUT std::vector<double> &areas) const = 0;
    CV_WRAP virtual void GetLength(CV_OUT std::vector<double> &lengthes) const = 0;
    CV_WRAP virtual void GetCentroid(CV_OUT std::vector<cv::Point2f> &centroids) const = 0;
    CV_WRAP virtual void GetBoundingBox(CV_OUT std::vector<cv::Rect> &boundingBoxes) const = 0;
    CV_WRAP virtual void GetSmallestCircle(CV_OUT std::vector< cv::Point3d> &miniCircles) const = 0;
    CV_WRAP virtual void GetCircularity(CV_OUT std::vector<double> &circularities) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> Simplify(const float tolerance) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetConvex() const = 0;
    CV_WRAP virtual int GetPoints(CV_OUT std::vector<cv::Point2f> &vertexes) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> Move(const cv::Point2f &delta) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> Zoom(const cv::Size2f &scale) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> AffineTrans(const cv::Matx33d &homoMat2D) const = 0;
    CV_WRAP virtual bool TestClosed() const = 0;
    CV_WRAP virtual bool TestConvex() const = 0;
    CV_WRAP virtual bool TestPoint(const cv::Point2f &point) const = 0;
    CV_WRAP virtual bool TestSelfIntersection(const cv::String &closeContour) const = 0;
    CV_WRAP virtual void GetTestClosed(CV_OUT std::vector<int> &isClosed) const = 0;
    CV_WRAP virtual void GetTestConvex(CV_OUT std::vector<int> &isConvex) const = 0;
    CV_WRAP virtual void GetTestPoint(const cv::Point2f &point, CV_OUT std::vector<int> &isInside) const = 0;
    CV_WRAP virtual void GetTestSelfIntersection(const cv::String &closeContour, CV_OUT std::vector<int> &doesIntersect) const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Scalar& color, const float thickness = 1, const int style = 0) const = 0;
};

}
}

#endif
