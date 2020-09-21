#ifndef __OPENCV_MVLAB_HDF5_GROUP_IMPL_HPP__
#define __OPENCV_MVLAB_HDF5_GROUP_IMPL_HPP__

#include <opencv2/mvlab/h5group.hpp>
#include <H5Cpp.h>

namespace cv {
namespace mvlab {

class H5GroupImpl : public H5Group
{
public:
    H5GroupImpl() {}
    H5GroupImpl(const cv::String &errMsg) : err_msg_(errMsg) {}
    H5GroupImpl(const H5::Group &group) : group_(group), err_msg_("OK") {}

public:
    bool Valid() const CV_OVERRIDE;
    cv::String GetErrorStatus() const CV_OVERRIDE;
    cv::Ptr<H5Group> GetGroup(const cv::String &name) const CV_OVERRIDE;
    int SetInt(const cv::String &name, const int val) CV_OVERRIDE;
    int SetMat(const cv::String &name, const cv::Mat &val) CV_OVERRIDE;
    int SetRegion(const cv::String &name, const cv::Ptr<Region> &val) CV_OVERRIDE;
    int SetContour(const cv::String &name, const cv::Ptr<Contour> &val) CV_OVERRIDE;
    int SetContourTemplate(const cv::String &name, const cv::Ptr<ContourTemplate> &val) CV_OVERRIDE;
    int SetPixelTemplate(const cv::String &name, const cv::Ptr<PixelTemplate> &val) CV_OVERRIDE;

public:
    int GetInt(const cv::String &name, const int defaultVal = -1) const CV_OVERRIDE;
    cv::Mat GetMat(const cv::String &name) const CV_OVERRIDE;
    cv::Ptr<Region> GetRegion(const cv::String &name) const CV_OVERRIDE;
    cv::Ptr<Contour> GetContour(const cv::String &name) const CV_OVERRIDE;
    cv::Ptr<ContourTemplate> GetContourTemplate(const cv::String &name) CV_OVERRIDE;
    cv::Ptr<PixelTemplate> GetPixelTemplate(const cv::String &name) CV_OVERRIDE;

public:
    H5::Group &Handle() { return group_; }
    H5::PredType GetH5type(const int cvType) const;
    void SetErrorMessage(const cv::String &errMsg) { err_msg_ = errMsg; }
    int GetCVtype(const H5::DataType &h5Type) const;
    int SetDataSet(const cv::String &name, const cv::Mat &val, H5::DataSet &dataSet);
    int SetDataSet(const cv::String &name, const std::string &bytes, H5::DataSet &dataSet);
    int GetDataSet(const cv::String &name, const cv::String &typeGUID, std::string &bytes, H5::DataSet &dataSet) const;
    int GetDataSet(const cv::String &name, cv::Mat &val, H5::DataSet &dataSet) const;
    int GetDataSet(const cv::String &name, const cv::String &typeGUID, cv::Mat &val, H5::DataSet &dataSet) const;
    int SetAttribute(const H5::H5Object &h5o, const cv::String &name, const int val);
    int GetAttribute(const H5::H5Object &h5o, const cv::String &name, int &val) const;
    int SetAttribute(const H5::H5Object &h5o, const cv::String &name, const long long val);
    int GetAttribute(const H5::H5Object &h5o, const cv::String &name, long long &val) const;
    int SetAttribute(const H5::H5Object &h5o, const cv::String &name, const cv::String &val);
    int GetAttribute(const H5::H5Object &h5o, const cv::String &name, cv::String &val) const;
    int SetAttribute(const H5::H5Object &h5o, const cv::String &name, const cv::Mat &val);
    int GetAttribute(const H5::H5Object &h5o, const cv::String &name, cv::Mat &val) const;

public:
    H5::Group group_;
    mutable cv::String err_msg_;
};

}
}

#endif //__OPENCV_MVLAB_HDF5_GROUP_IMPL_HPP__
