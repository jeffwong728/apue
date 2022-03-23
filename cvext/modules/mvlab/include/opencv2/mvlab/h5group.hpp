#ifndef __OPENCV_MVLAB_HDF5_GROUP_HPP__
#define __OPENCV_MVLAB_HDF5_GROUP_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W H5Group
{
public:
    H5Group() {}
    virtual ~H5Group() {}

public:
    CV_WRAP virtual bool Valid() const = 0;
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;
    CV_WRAP virtual cv::Ptr<H5Group> GetGroup(const cv::String &name) const = 0;
    CV_WRAP virtual int SetInt(const cv::String &name, const int val) = 0;
    CV_WRAP virtual int SetMat(const cv::String &name, const cv::Mat &val) = 0;
    CV_WRAP virtual int SetRegion(const cv::String &name, const cv::Ptr<Region> &val) = 0;
    CV_WRAP virtual int SetContour(const cv::String &name, const cv::Ptr<Contour> &val) = 0;
    CV_WRAP virtual int SetContourTemplate(const cv::String &name, const cv::Ptr<ContourTemplate> &val) = 0;
    CV_WRAP virtual int SetPixelTemplate(const cv::String &name, const cv::Ptr<PixelTemplate> &val) = 0;
    CV_WRAP virtual int GetInt(const cv::String &name, const int defaultVal = -1) const = 0;
    CV_WRAP virtual cv::Mat GetMat(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Ptr<Region> GetRegion(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetContour(const cv::String &name) const = 0;
    CV_WRAP virtual cv::Ptr<ContourTemplate> GetContourTemplate(const cv::String &name) = 0;
    CV_WRAP virtual cv::Ptr<PixelTemplate> GetPixelTemplate(const cv::String &name) = 0;
};

}
}

#endif //__OPENCV_MVLAB_HDF5_GROUP_HPP__
