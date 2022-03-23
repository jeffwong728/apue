#ifndef __OPENCV_MVLAB_DICT_IMPL_HPP__
#define __OPENCV_MVLAB_DICT_IMPL_HPP__

#include <opencv2/mvlab/dict.hpp>
#include <map>

namespace cv {
namespace mvlab {

class DictImpl : public Dict
{
public:
    DictImpl() { }

public:
    int SetInt(const cv::String &name, const int val) CV_OVERRIDE;
    int SetReal64(const cv::String &name, const double val) CV_OVERRIDE;
    int SetReal32(const cv::String &name, const float val) CV_OVERRIDE;
    int SetString(const cv::String &name, const cv::String &val) CV_OVERRIDE;
    int SetMat(const cv::String &name, const cv::Mat &val) CV_OVERRIDE;
    int SetRegion(const cv::String &name, const cv::Ptr<Region> &val) CV_OVERRIDE;
    int SetScalar(const cv::String &name, const cv::Scalar &val) CV_OVERRIDE;
    int SetH5Group(const cv::String &name, const cv::Ptr<H5Group> &val) CV_OVERRIDE;

public:
    bool Empty() const CV_OVERRIDE;
    int GetInt(const cv::String &name, const int defaultVal) const CV_OVERRIDE;
    double GetReal64(const cv::String &name, const double defaultVal) const CV_OVERRIDE;
    float GetReal32(const cv::String &name, const float defaultVal) const CV_OVERRIDE;
    cv::String GetString(const cv::String &name) const CV_OVERRIDE;
    cv::Mat GetMat(const cv::String &name) const CV_OVERRIDE;
    cv::Ptr<Region> GetRegion(const cv::String &name) const CV_OVERRIDE;
    cv::Scalar GetScalar(const cv::String &name, const cv::Scalar &defaultVal) const CV_OVERRIDE;
    cv::Ptr<H5Group> GetH5Group(const cv::String &name) const CV_OVERRIDE;

private:
    std::map<cv::String, int> ints_;
    std::map<cv::String, double> reals_;
    std::map<cv::String, float> singles_;
    std::map<cv::String, cv::Scalar> scalars_;
    std::map<cv::String, cv::String> strs_;
    std::map<cv::String, cv::Ptr<Region>> rgns_;
    std::map<cv::String, cv::Ptr<H5Group>> groups_;
    std::map<cv::String, cv::Mat> mats_;
};

}
}

#endif //__OPENCV_MVLAB_DICT_IMPL_HPP__
