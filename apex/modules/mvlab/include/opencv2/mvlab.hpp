#ifndef __OPENCV_ARUCO_HPP__
#define __OPENCV_ARUCO_HPP__

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

class CV_EXPORTS_W Board {

    public:
    CV_WRAP static Ptr<Board> create(InputArrayOfArrays objPoints, const Ptr<Region> &dictionary, InputArray ids);
    CV_PROP std::vector< std::vector< Point3f > > objPoints;
    CV_PROP Ptr<Region> dictionary;
    CV_PROP std::vector< int > ids;
};
}
}

#endif
