#ifndef __OPENCV_MVLAB_HDF5_DATABASE_HPP__
#define __OPENCV_MVLAB_HDF5_DATABASE_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W H5DB
{
public:
    H5DB() {}
    virtual ~H5DB() {}

    CV_WRAP static cv::Ptr<H5DB> Open(const cv::String &HDF5Filename);

public:
    CV_WRAP virtual bool Valid() const = 0;
    CV_WRAP virtual void Close() = 0;
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;
    CV_WRAP virtual cv::Ptr<H5Group> GetRoot() const = 0;
    CV_WRAP virtual cv::Ptr<H5Group> GetGroup(const cv::String &name) const = 0;
};

}
}

#endif // __OPENCV_MVLAB_HDF5_DATABASE_HPP__
