#ifndef __OPENCV_MVLAB_HPP__
#define __OPENCV_MVLAB_HPP__

#include <opencv2/core.hpp>
#include "opencv2/mvlab/region.hpp"
#include "opencv2/mvlab/contour.hpp"

namespace cv {
namespace mvlab {

constexpr int MLR_OK = 0;
constexpr int MLR_SUCCESS = MLR_OK;
constexpr int MLR_IMAGE_EMPTY = 10000;
constexpr int MLR_REGION_EMPTY = 20000;
constexpr int MLR_ERROR = -1;
constexpr int MLR_FAILURE = MLR_ERROR;

CV_EXPORTS_W int SetGlobalOption(const String& optName, const String& optVal);
CV_EXPORTS_W int GetGlobalOption(const String& optName, CV_OUT String& optVal);

}
}

#endif //__OPENCV_MVLAB_HPP__
