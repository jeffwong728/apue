#ifndef __OPENCV_MVLAB_REGION_HPP__
#define __OPENCV_MVLAB_REGION_HPP__

#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W Region {

public:
    Region() {}
    virtual ~Region() {}

    CV_WRAP static Ptr<Region> CreateRectangle(const Rect &rectSize);

public:
    CV_WRAP virtual double Area() const = 0;
    CV_WRAP virtual Point2d Centroid() const = 0;
    CV_WRAP virtual void Connect(CV_OUT std::vector<Ptr<Region>> &regions) const = 0;
};

}
}

#endif
