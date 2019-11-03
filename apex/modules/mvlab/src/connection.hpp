#ifndef __OPENCV_MVLAB_CONNECTION_HPP__
#define __OPENCV_MVLAB_CONNECTION_HPP__

#include <opencv2/mvlab/region.hpp>
#include <opencv2/mvlab/region_collection.hpp>

namespace cv {
namespace mvlab {

class ConnectWuSerial
{
public:
    ConnectWuSerial() {}
public:
    cv::Ptr<RegionCollection> operator()(const Region *rgn, const int connectivity) const;
};

}
}

#endif //__OPENCV_MVLAB_CONNECTION_HPP__
