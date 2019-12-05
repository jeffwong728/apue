#ifndef __OPENCV_MVLAB_CONVEX_HULL_HPP__
#define __OPENCV_MVLAB_CONVEX_HULL_HPP__

#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/core/noncopyable.hpp>

namespace cv {
namespace mvlab {

class ConvexHull : private boost::noncopyable
{
public:
    ConvexHull() = delete;

public:
    static ScalablePoint2fSequence Sklansky(const cv::Point2f *points, const int cPoints);
    static ScalablePoint2fSequence AndrewMonotoneChain(const cv::Point2f *points, const int cPoints);
    static ScalablePoint2fSequence MelkmanSimpleHull(const cv::Point2f *points, const int cPoints);
};

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
