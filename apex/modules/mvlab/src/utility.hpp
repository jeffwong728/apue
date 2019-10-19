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
    static cv::Mat PathToMask(const Geom::PathVector &pv, const cv::Size &sz, std::vector<uint8_t> &buf);
};

}
}

#endif //__OPENCV_MVLAB_UTILITY_HPP__
