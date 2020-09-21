#ifndef __OPENCV_MVLAB_MATCH_RESULT_HPP__
#define __OPENCV_MVLAB_MATCH_RESULT_HPP__

#include "forward.hpp"
#include <opencv2/core.hpp>

namespace cv {
namespace mvlab {

class CV_EXPORTS_W MatchResult {

public:
    MatchResult() {}
    virtual ~MatchResult() {}

    /** @brief Get template error status message.
    */
    CV_WRAP virtual cv::String GetErrorStatus() const = 0;
    CV_WRAP virtual int Count() const = 0;
    CV_WRAP virtual float GetScore() const = 0;
    CV_WRAP virtual cv::Point2f GetPosition() const = 0;
    CV_WRAP virtual float GetAngle() const = 0;
    CV_WRAP virtual cv::Size2f GetScale() const = 0;
    CV_WRAP virtual cv::Matx33d GetAffine() const = 0;
    CV_WRAP virtual float GetNthScore(const int nth) const = 0;
    CV_WRAP virtual cv::Point2f GetNthPosition(const int nth) const = 0;
    CV_WRAP virtual float GetNthAngle(const int nth) const = 0;
    CV_WRAP virtual cv::Size2f GetNthScale(const int nth) const = 0;
    CV_WRAP virtual cv::Matx33d GetNthAffine(const int nth) const = 0;
    CV_WRAP virtual int Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts = nullptr) const = 0;
    CV_WRAP virtual int DrawNth(cv::InputOutputArray img, const int nth, const cv::Ptr<Dict> &opts = nullptr) const = 0;
};

}
}

#endif //__OPENCV_MVLAB_MATCH_RESULT_HPP__
