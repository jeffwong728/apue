#ifndef __OPENCV_MVLAB_TRANSFORMATIONS_HPP__
#define __OPENCV_MVLAB_TRANSFORMATIONS_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W HomoMat2d {

public:
    HomoMat2d() {}
    virtual ~HomoMat2d() {}

    CV_WRAP static cv::Matx33d GenIdentity();
    CV_WRAP static cv::Matx33d Translate(const cv::Matx33d &homoMat2D, const cv::Point2d &txy);
    CV_WRAP static cv::Matx33d TranslateLocal(const cv::Matx33d &homoMat2D, const cv::Point2d &txy);
    CV_WRAP static cv::Matx33d Rotate(const cv::Matx33d &homoMat2D, const double phi, const cv::Point2d &pxy);
    CV_WRAP static cv::Matx33d RotateLocal(const cv::Matx33d &homoMat2D, const double phi);
    CV_WRAP static cv::Matx33d Scale(const cv::Matx33d &homoMat2D, const cv::Size2d &sxy, const cv::Point2d &pxy);
    CV_WRAP static cv::Matx33d ScaleLocal(const cv::Matx33d &homoMat2D, const cv::Size2d &sxy);
    CV_WRAP static cv::Matx33d Slant(const cv::Matx33d &homoMat2D, const double theta, const cv::Point2d &pxy, const cv::String &axis);
    CV_WRAP static cv::Matx33d SlantLocal(const cv::Matx33d &homoMat2D, const double theta, const cv::String &axis);
    CV_WRAP static cv::Matx33d Compose(const cv::Matx33d &homoMat2DLeft, const cv::Matx33d &homoMat2DRight);
    CV_WRAP static cv::Matx33d Invert(const cv::Matx33d &homoMat2D);
    CV_WRAP static cv::Matx33d Reflect(const cv::Matx33d &homoMat2D, const cv::Point2d &pxy, const cv::Point2d &qxy);
    CV_WRAP static cv::Matx33d ReflectLocal(const cv::Matx33d &homoMat2D, const cv::Point2d &pxy);
    CV_WRAP static cv::Matx33d VectorAngleToRigid(const cv::Point2d &point1, const double angle1, const cv::Point2d &point2, const double angle2);
    CV_WRAP static double Determinant(const cv::Matx33d &homoMat2D);
    CV_WRAP static cv::Point2d AffineTransPixel(const cv::Matx33d &homoMat2D, const cv::Point2d &pixel);
    CV_WRAP static cv::Point2d AffineTransPoint2d(const cv::Matx33d &homoMat2D, const cv::Point2d &point);
};

}
}

#endif //__OPENCV_MVLAB_TRANSFORMATIONS_HPP__
