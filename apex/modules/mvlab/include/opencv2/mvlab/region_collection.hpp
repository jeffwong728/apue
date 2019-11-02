#ifndef __OPENCV_MVLAB_REGION_COLLECTION_HPP__
#define __OPENCV_MVLAB_REGION_COLLECTION_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W RegionCollection {

public:
    RegionCollection() {}
    virtual ~RegionCollection() {}

public:
    virtual int Draw(Mat &img, const Scalar& color, const float thickness = 1, const int style = 0) const = 0;

public:
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual void Area(CV_OUT std::vector<double> &areas) const = 0;
    CV_WRAP virtual void Length(CV_OUT std::vector<double> &lengths) const = 0;
    CV_WRAP virtual void Centroid(CV_OUT std::vector<cv::Point2d> &centroids) const = 0;
    CV_WRAP virtual void BoundingBox(CV_OUT std::vector<cv::Rect> &bboxs) const = 0;
    CV_WRAP virtual int Draw(InputOutputArray img, const Scalar& color, const float thickness = 1, const int style = 0) const = 0;
};

}
}

#endif //__OPENCV_MVLAB_REGION_COLLECTION_HPP__
