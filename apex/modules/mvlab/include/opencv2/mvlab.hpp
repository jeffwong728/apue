#ifndef __OPENCV_MVLAB_HPP__
#define __OPENCV_MVLAB_HPP__

#include <opencv2/core.hpp>
#include <opencv2/mvlab/dict.hpp>
#include <opencv2/mvlab/h5db.hpp>
#include <opencv2/mvlab/h5group.hpp>
#include <opencv2/mvlab/cmndef.hpp>
#include <opencv2/mvlab/region.hpp>
#include <opencv2/mvlab/contour.hpp>
#include <opencv2/mvlab/transformations.hpp>
#include <opencv2/mvlab/contour_template.hpp>

namespace cv {
namespace mvlab {

CV_EXPORTS_W int Initialize(const cv::String& fileName);
CV_EXPORTS_W int SetGlobalOption(const cv::String& optName, const cv::String& optVal);
CV_EXPORTS_W int GetGlobalOption(const cv::String& optName, CV_OUT cv::String& optVal);
CV_EXPORTS_W int Threshold(cv::InputArray src, const int minGray, const int maxGray, CV_OUT cv::Ptr<Region> &region);

}
}

#endif //__OPENCV_MVLAB_HPP__