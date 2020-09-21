#ifndef __OPENCV_MVLAB_DICT_HPP__
#define __OPENCV_MVLAB_DICT_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Dict
{
public:
    Dict() {}
    virtual ~Dict() {}

    CV_WRAP static cv::Ptr<Dict> GenEmpty();

public:
    CV_WRAP virtual int SetInt(const cv::String &name, const int val) = 0;
    CV_WRAP virtual int SetReal64(const cv::String &name, const double val) = 0;
    CV_WRAP virtual int SetReal32(const cv::String &name, const float val) = 0;
    CV_WRAP virtual int SetString(const cv::String &name, const cv::String &val) = 0;
    CV_WRAP virtual int SetMat(const cv::String &name, const cv::Mat &val) = 0;
    CV_WRAP virtual int SetRegion(const cv::String &name, const cv::Ptr<Region> &val) = 0;
    CV_WRAP virtual int SetScalar(const cv::String &name, const cv::Scalar &val) = 0;
    CV_WRAP virtual int SetH5Group(const cv::String &name, const cv::Ptr<H5Group> &val) = 0;

public:
    CV_WRAP virtual bool Empty() const = 0;
    CV_WRAP virtual int GetInt(const cv::String &name, const int defaultVal = -1) const = 0;
    CV_WRAP virtual double GetReal64(const cv::String &name, const double defaultVal = -1.) const = 0;
    CV_WRAP virtual float GetReal32(const cv::String &name, const float defaultVal = -1.f) const = 0;
    CV_WRAP virtual cv::String GetString(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Mat GetMat(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Ptr<Region> GetRegion(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Scalar GetScalar(const cv::String &name, const cv::Scalar &defaultVal = cv::Scalar()) const = 0;
    CV_WRAP virtual cv::Ptr<H5Group> GetH5Group(const cv::String &name) const = 0;
};

}
}

#endif // __OPENCV_MVLAB_DICT_HPP__
