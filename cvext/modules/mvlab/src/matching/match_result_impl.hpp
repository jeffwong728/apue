#ifndef __OPENCV_MVLAB_MATCH_RESULT_IMPL_HPP__
#define __OPENCV_MVLAB_MATCH_RESULT_IMPL_HPP__

#include <opencv2/mvlab/match_result.hpp>

namespace cv {
namespace mvlab {
class PixelTemplateImpl;
class ContourTemplateImpl;
class MatchResultImpl : public MatchResult
{
public:
    MatchResultImpl(const cv::Ptr<PixelTemplateImpl> &tmpl) : pixel_template_(tmpl) {}
    MatchResultImpl(const cv::Ptr<ContourTemplateImpl> &tmpl) : contour_template_(tmpl) {}

public:
    cv::String GetErrorStatus() const CV_OVERRIDE;
    int Count() const CV_OVERRIDE;
    float GetScore() const CV_OVERRIDE;
    cv::Point2f GetPosition() const CV_OVERRIDE;
    float GetAngle() const CV_OVERRIDE;
    cv::Size2f GetScale() const CV_OVERRIDE;
    cv::Matx33d GetAffine() const CV_OVERRIDE;
    float GetNthScore(const int nth) const CV_OVERRIDE;
    cv::Point2f GetNthPosition(const int nth) const CV_OVERRIDE;
    float GetNthAngle(const int nth) const CV_OVERRIDE;
    cv::Size2f GetNthScale(const int nth) const CV_OVERRIDE;
    cv::Matx33d GetNthAffine(const int nth) const CV_OVERRIDE;
    int Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;
    int DrawNth(cv::InputOutputArray img, const int nth, const cv::Ptr<Dict> &opts) const CV_OVERRIDE;

public:
    void SetErrorMessage(const cv::String &errMsg) { err_msg_.resize(0);  err_msg_ = errMsg; }
    void Reserve(const int numInstance)
    {
        scores_.reserve(numInstance);
        positions_.reserve(numInstance);
        angles_.reserve(numInstance);
        scales_.reserve(numInstance);
    }

    void AddInstance(const float score, const cv::Point2f &pos, const float angle) { AddInstance(score, pos, angle, cv::Size2f(1.f, 1.f)); }
    void AddInstance(const float score, const cv::Point2f &pos, const float angle, const cv::Size2f &scale)
    {
        scores_.push_back(score);
        positions_.push_back(pos);
        angles_.push_back(angle);
        scales_.push_back(scale);
    }

private:
    mutable cv::String err_msg_;
    ScalableFloatSequence scores_;
    ScalablePoint2fSequence positions_;
    ScalableFloatSequence angles_;
    ScalableSize2fSequence scales_;
    cv::Ptr<PixelTemplateImpl>   pixel_template_;
    cv::Ptr<ContourTemplateImpl> contour_template_;
};

}
}

#endif //__OPENCV_MVLAB_MATCH_RESULT_IMPL_HPP__
