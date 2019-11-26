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
    static inline float deg(const float angle) { return angle * 180.f / static_cast<float>(CV_PI); }
    static inline float dist(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline float dist(const cv::Point2f &p0, const cv::Point2f &p1);
    static inline cv::Point2f midPoint(const cv::Point2f *p0, const cv::Point2f *p1);
    static inline cv::Point2f interPoint(const float t, const cv::Point2f *p0, const cv::Point2f *p1);
    static inline bool nearPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
    static inline bool farPoint(const cv::Point2f *p0, const cv::Point2f *p1, const float tol);
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

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
