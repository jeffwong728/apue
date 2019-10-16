#ifndef __OPENCV_MVLAB_HPP__
#define __OPENCV_MVLAB_HPP__

#include <opencv2/core.hpp>
#include <vector>
#include "opencv2/mvlab/region.hpp"

namespace cv {
namespace mvlab {

enum CornerRefineMethod{
    CORNER_REFINE_NONE,
    CORNER_REFINE_SUBPIX,
    CORNER_REFINE_CONTOUR,
    CORNER_REFINE_APRILTAG
};

CV_EXPORTS_W int SetGlobalOption(const String& optName, const String& optVal);
CV_EXPORTS_W int GetGlobalOption(const String& optName, CV_OUT String& optVal);

}
}

#endif //__OPENCV_MVLAB_HPP__
