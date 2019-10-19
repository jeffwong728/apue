#ifndef __OPENCV_MVLAB_CONTOUR_HPP__
#define __OPENCV_MVLAB_CONTOUR_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Contour {

public:
    Contour() {}
    virtual ~Contour() {}

    CV_WRAP static Ptr<Contour> CreateEmpty();
    CV_WRAP static Ptr<Contour> CreateRectangle(const Rect2f &rect);
    CV_WRAP static Ptr<Contour> CreateRotatedRectangle(const RotatedRect &rotatedRect);
    CV_WRAP static Ptr<Contour> CreateCircle(const Point2f &center, const float radius);
    CV_WRAP static Ptr<Contour> CreateEllipse(const Point2f &center, const Size2f &size);
    CV_WRAP static Ptr<Contour> CreateRotatedEllipse(const Point2f &center, const Size2f &size, const float angle);

public:
    virtual int Draw(Mat &img, const Scalar& color, const float thickness = 1, const int style = 0) const = 0;

public:
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual double Length() const = 0;
    CV_WRAP virtual Point2d Centroid() const = 0;
    CV_WRAP virtual Rect BoundingBox() const = 0;
    CV_WRAP virtual int Draw(InputOutputArray img, const Scalar& color, const float thickness = 1, const int style = 0) const = 0;
};

}
}

#endif
