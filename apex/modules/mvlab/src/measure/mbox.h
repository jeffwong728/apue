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
    MeasureBoxImpl(const cv::RotatedRect &box, const cv::Size2f &sampleSize);

public:
    bool Valid() const CV_OVERRIDE { return NumLines() >= 3; }
    int SetSigma(const float sigma) const CV_OVERRIDE;
    cv::Ptr<Contour> GetMarks() const CV_OVERRIDE;
    int GetProfile(cv::InputArray img, std::vector<double> &grays) const CV_OVERRIDE;
    template<typename T> int GetProfileImpl(cv::InputArray img, std::vector<T> &grays) const;

private:
    int NumLines() const
    {
        return cvRound(lengths_.width / sample_size_.width);
    }
    void Smooth(std::vector<double> &grays) const;

private:
    cv::RotatedRect box_;
    cv::Point2f start_point_;
    cv::Point2f end_point_;
    cv::Size2f  sample_size_;
    cv::Size2f  lengths_;
    cv::Point2f delta_1_;
    cv::Point2f delta_2_;
    mutable std::vector<float> g_kernel_;
};

}
}

#endif //MVLAB_SRC_MEASURE_BOX_H