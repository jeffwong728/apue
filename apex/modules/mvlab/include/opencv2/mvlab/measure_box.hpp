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

    CV_WRAP static cv::Ptr<MeasureBox> Gen(const cv::RotatedRect &box, const cv::Size2f &sampleSize);

public:
    CV_WRAP virtual bool Valid() const = 0;
    CV_WRAP virtual int SetSigma(const float sigma) const = 0;
    CV_WRAP virtual cv::Ptr<Contour> GetMarks() const = 0;
    CV_WRAP virtual int GetProfile(cv::InputArray img, CV_OUT std::vector<double> &grays) const = 0;
};

}
}

#endif // __OPENCV_MVLAB_MEASURE_BOX_HPP__
