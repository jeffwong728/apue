#ifndef __OPENCV_MVLAB_MEASURE_BOX_HPP__
#define __OPENCV_MVLAB_MEASURE_BOX_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W MeasureBox
{
public:
    MeasureBox() {}
    virtual ~MeasureBox() {}

    CV_WRAP static cv::Ptr<MeasureBox> GenMeasureBox(const cv::RotatedRect &box, const cv::Point2f &sampleSize);

public:
    CV_WRAP virtual bool Valid() const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetMarks() const = 0;
};

}
}

#endif // __OPENCV_MVLAB_MEASURE_BOX_HPP__
