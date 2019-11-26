#ifndef __OPENCV_MVLAB_CONTOUR_HPP__
#define __OPENCV_MVLAB_CONTOUR_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Contour {

public:
    Contour() {}
    virtual ~Contour() {}

    CV_WRAP static Ptr<Contour> GenEmpty();
    CV_WRAP static Ptr<Contour> GenRectangle(const cv::Rect2f &rect);
    CV_WRAP static Ptr<Contour> GenRotatedRectangle(const cv::RotatedRect &rotatedRect);
    CV_WRAP static Ptr<Contour> GenCircle(const cv::Point2f &center, const float radius, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenCircleSector(const cv::Point2f &center, const float radius, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenEllipse(const cv::Point2f &center, const cv::Size2f &size, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenRotatedEllipse(const cv::Point2f &center, const cv::Size2f &size, const float angle, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenRotatedEllipseSector(const cv::Point2f &center, const cv::Size2f &size, const float angle, const float startAngle, const float endAngle, const float resolution, const cv::String &specification);
    CV_WRAP static Ptr<Contour> GenPolygon(const std::vector<cv::Point2f> &vertexes);
    CV_WRAP static Ptr<Contour> GenPolyline(const std::vector<cv::Point2f> &vertexes);
    CV_WRAP static Ptr<Contour> GenPolygonRounded(const std::vector<cv::Point2f> &vertexes, const std::vector<cv::Size2f> &radius, const float samplingInterval);

public:
    virtual int Draw(Mat &img, const Scalar& color, const float thickness = 1, const int style = 0) const = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual double Length() const = 0;
    CV_WRAP virtual cv::Point2d Centroid() const = 0;
    CV_WRAP virtual cv::Rect BoundingBox() const = 0;
    CV_WRAP virtual Ptr<Contour> Simplify(const float tolerance) const = 0;
    CV_WRAP virtual int GetPoints(CV_OUT std::vector<cv::Point2f> &vertexes) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> Move(const cv::Point2f &delta) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> Zoom(const cv::Size2f &scale) const = 0;
    CV_WRAP virtual bool TestClosed() const = 0;
    CV_WRAP virtual bool TestPoint(const cv::Point2f &point) const = 0;
    CV_WRAP virtual bool TestSelfIntersection() const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Scalar& color, const float thickness = 1, const int style = 0) const = 0;
};

}
}

#endif
