#ifndef __OPENCV_MVLAB_ROT_CALIPERS_HPP__
#define __OPENCV_MVLAB_ROT_CALIPERS_HPP__

#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab.hpp>
#include <boost/core/noncopyable.hpp>

namespace cv {
namespace mvlab {

class RotatingCaliper : private boost::noncopyable
{
public:
    RotatingCaliper() = delete;

public:
    static cv::RotatedRect minAreaRect(const cv::Point2f *points, const int cPoints);
    static cv::Scalar diameterBruteForce(const cv::Point2f *points, const int cPoints);
};

}
}

#endif //__OPENCV_MVLAB_ROT_CALIPERS_HPP__
