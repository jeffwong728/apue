#ifndef __OPENCV_MVLAB_HDF5_DATABASE_IMPL_HPP__
#define __OPENCV_MVLAB_HDF5_DATABASE_IMPL_HPP__

#include <opencv2/mvlab/h5db.hpp>
#include <H5Cpp.h>

namespace cv {
namespace mvlab {

class H5DBImpl : public H5DB
{
public:
    H5DBImpl(const cv::String &errMsg) : err_msg_(errMsg) { }
    H5DBImpl(const H5::H5File &file) : file_(file), err_msg_("OK") {}

public:
    bool Valid() const CV_OVERRIDE;
    void Close() CV_OVERRIDE;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    cv::Ptr<H5Group> GetRoot() const CV_OVERRIDE;
    cv::Ptr<H5Group> GetGroup(const cv::String &name) const CV_OVERRIDE;

private:
    H5::H5File file_;
    cv::String err_msg_;
};

}
}

#endif //__OPENCV_MVLAB_HDF5_DATABASE_IMPL_HPP__
