#ifndef MVLAB_SRC_MEASURE_BOX_H
#define MVLAB_SRC_MEASURE_BOX_H
#include <vector>
#include <forward_list>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/mvlab/measure_box.hpp>
#include <boost/core/noncopyable.hpp>
#include <tbb/scalable_allocator.h>

namespace cv
{
namespace mvlab
{

class MeasureBoxImpl : public MeasureBox
{
public:
    MeasureBoxImpl(const cv::RotatedRect &box, const cv::Point2f &sampleSize);

public:
    bool Valid() const CV_OVERRIDE { return NumLines() >= 3; }
    cv::Ptr<Contour> GetMarks() const CV_OVERRIDE;
    int GetProfile(cv::InputArray img, std::vector<double> &grays) const CV_OVERRIDE;

private:
    int NumLines() const { return cvRound(lengths_.x / sample_size_.x); }

private:
    cv::RotatedRect box_;
    cv::Point2f start_point_;
    cv::Point2f end_point_;
    cv::Point2f sample_size_;
    cv::Point2f lengths_;
    cv::Point2f delta_1_;
    cv::Point2f delta_2_;
};

}
}

#endif //MVLAB_SRC_MEASURE_BOX_H