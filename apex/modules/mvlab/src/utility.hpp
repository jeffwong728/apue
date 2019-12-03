#ifndef __OPENCV_MVLAB_UTILITY_HPP__
#define __OPENCV_MVLAB_UTILITY_HPP__

#include <2geom/2geom.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/core/noncopyable.hpp>

namespace cv {
namespace mvlab {

class Util : private boost::noncopyable
{
public:
    Util() = delete;

public:
    static std::vector<double> GetDashesPattern(const int bls, const double lineWidth);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz);
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz, UScalableUCharSequence &buf);
    static inline float constrainAngle(float x);
    static inline float square(float x) { return x * x; }
    static inline float rad(const float angle) { return angle * static_cast<float>(CV_PI) / 180.f; }
    static inline double rad(const double angle) { return angle * CV_PI / 180.; }
    static inline float deg(const float angle) { return angle * 180.f / static_cast<float>(CV_PI); }
    static inline float dist(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline float dist(const cv::Point2f &p0, const cv::Point2f &p1);
    static inline cv::Point2f midPoint(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1);
    static inline bool nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline bool farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline cv::Point changedToFixed(const cv::Point2f &point);
    static inline int isLeft(const cv::Point &P0, const cv::Point &P1, const cv::Point &P2);
    static inline float isLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2);
    static inline int isLeft(const int P0x, const int P0y, const int P1x, const int P1y, const int P2x, const int P2y);
    static inline vcl::Vec8f isLeft(const vcl::Vec8f &P0x, const vcl::Vec8f &P0y, const vcl::Vec8f &P1x, const vcl::Vec8f &P1y, const vcl::Vec8f &P2x, const vcl::Vec8f &P2y);
};

inline float Util::constrainAngle(float x)
{
    x = std::fmod(x, 360.f);
    if (x < 0)
        x += 360;
    return x;
}

inline float Util::dist(const cv::Point2f *p0, const cv::Point2f *p1)
{
    return std::sqrtf(square(p1->x-p0->x) + square(p1->y - p0->y));
}

inline float Util::dist(const cv::Point2f &p0, const cv::Point2f &p1)
{
    return std::sqrtf(square(p1.x - p0.x) + square(p1.y - p0.y));
}

inline cv::Point2f Util::midPoint(const cv::Point2f *p0, const cv::Point2f *p1)
{
    return cv::Point2f((p0->x + p1->x) / 2, (p0->y + p1->y) / 2);
}

inline cv::Point2f Util::interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1)
{
    return t * (*p0) + (1 - t) * (*p1);
}

inline bool Util::nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol)
{
    return (square(p1->x - p0->x) + square(p1->y - p0->y)) < tol;
}

inline bool Util::farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol)
{
    return (square(p1->x - p0->x) + square(p1->y - p0->y)) > tol;
}

inline cv::Point Util::changedToFixed(const cv::Point2f &point)
{
    return cv::Point(cvRound(point.x * F_XY_ONE), cvRound(point.y * F_XY_ONE));
}

inline int Util::isLeft(const cv::Point &P0, const cv::Point &P1, const cv::Point &P2)
{
    return (P2.x - P0.x) * (P1.y - P0.y) - (P1.x - P0.x) * (P2.y - P0.y);
}

inline float Util::isLeft(const cv::Point2f &P0, const cv::Point2f &P1, const cv::Point2f &P2)
{
    return (P2.x - P0.x) * (P1.y - P0.y) - (P1.x - P0.x) * (P2.y - P0.y);
}

inline int Util::isLeft(const int P0x, const int P0y, const int P1x, const int P1y, const int P2x, const int P2y)
{
    return (P2x - P0x) * (P1y - P0y) - (P1x - P0x) * (P2y - P0y);
}

inline vcl::Vec8f Util::isLeft(const vcl::Vec8f &P0x, const vcl::Vec8f &P0y,
    const vcl::Vec8f &P1x, const vcl::Vec8f &P1y,
    const vcl::Vec8f &P2x, const vcl::Vec8f &P2y)
{
    return (P2x - P0x) * (P1y - P0y) - (P1x - P0x) * (P2y - P0y);
}

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
