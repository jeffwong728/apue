#ifndef __OPENCV_MVLAB_HPP__
#define __OPENCV_MVLAB_HPP__

#include <opencv2/core.hpp>
#include <opencv2/mvlab/region.hpp>
#include <opencv2/mvlab/contour.hpp>
#include <opencv2/mvlab/transformations.hpp>

namespace cv {
namespace mvlab {

constexpr int MLR_OK                                            = 0;
constexpr int MLR_SUCCESS                                       = MLR_OK;
constexpr int MLR_CPU_UNSUPPORTED                               = 1;
constexpr int MLR_MEMORY_ERROR                                  = 2;
constexpr int MLR_PARAMETER_ERROR                               = 5000;
constexpr int MLR_PARAMETER_ERROR_GUARD                         = 5100;
constexpr int MLR_IMAGE_EMPTY                                   = 10000;
constexpr int MLR_IMAGE_FORMAT_ERROR                            = 10001;
constexpr int MLR_REGION_EMPTY                                  = 11000;
constexpr int MLR_CONTOUR_EMPTY                                 = 12000;
constexpr int MLR_ERROR                                         = -1;
constexpr int MLR_FAILURE                                       = MLR_ERROR;

enum BoundaryLineStyle {
    BOUNDARY_LINE_SOLID = 0,
    BOUNDARY_LINE_DASH = 1,
    BOUNDARY_LINE_DOT = 2,
    BOUNDARY_LINE_DASHDOT = 3,
    BOUNDARY_LINE_DASHDOTDOT = 4,
    BOUNDARY_LINE_CUSTOM = 5
};

CV_EXPORTS_W int Initialize(const cv::String& fileName);
CV_EXPORTS_W int SetGlobalOption(const cv::String& optName, const cv::String& optVal);
CV_EXPORTS_W int GetGlobalOption(const cv::String& optName, CV_OUT cv::String& optVal);
CV_EXPORTS_W int Threshold(cv::InputArray src, const int minGray, const int maxGray, CV_OUT cv::Ptr<Region> &region);

}
}

#endif //__OPENCV_MVLAB_HPP__