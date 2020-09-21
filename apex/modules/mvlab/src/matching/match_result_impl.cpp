#include "precomp.hpp"
#include "match_result_impl.hpp"
#include "pixel_template_impl.hpp"
#include "contour_template_impl.hpp"
#include "utility.hpp"
#include <hdf5/h5group_impl.hpp>
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

cv::String MatchResultImpl::GetErrorStatus() const
{
    return err_msg_;
}

int MatchResultImpl::Count() const
{
    return static_cast<int>(scores_.size());
}

float MatchResultImpl::GetScore() const
{
    if (scores_.empty())
    {
        return 0.f;
    }
    else
    {
        return scores_.front();
    }
}

cv::Point2f MatchResultImpl::GetPosition() const
{
    if (positions_.empty())
    {
        return cv::Point2f();
    }
    else
    {
        return positions_.front();
    }
}

float MatchResultImpl::GetAngle() const
{
    if (angles_.empty())
    {
        return 0.f;
    }
    else
    {
        return angles_.front();
    }
}

cv::Size2f MatchResultImpl::GetScale() const
{
    if (scales_.empty())
    {
        return cv::Size2f();
    }
    else
    {
        return scales_.front();
    }
}

cv::Matx33d MatchResultImpl::GetAffine() const
{
    return HomoMat2d::GenIdentity();
}

float MatchResultImpl::GetNthScore(const int nth) const
{
    if (nth >= 0 && nth < static_cast<int>(scores_.size()))
    {
        return scores_[nth];
    }
    else
    {
        return 0.f;
    }
}

cv::Point2f MatchResultImpl::GetNthPosition(const int nth) const
{
    if (nth >= 0 && nth < static_cast<int>(positions_.size()))
    {
        return positions_[nth];
    }
    else
    {
        return cv::Point2f();
    }
}

float MatchResultImpl::GetNthAngle(const int nth) const
{
    if (nth >= 0 && nth < static_cast<int>(angles_.size()))
    {
        return angles_[nth];
    }
    else
    {
        return 0.f;
    }
}

cv::Size2f MatchResultImpl::GetNthScale(const int nth) const
{
    if (nth >= 0 && nth < static_cast<int>(scales_.size()))
    {
        return scales_[nth];
    }
    else
    {
        return cv::Size2f();
    }
}

cv::Matx33d MatchResultImpl::GetNthAffine(const int nth) const
{
    return HomoMat2d::GenIdentity();
}

int MatchResultImpl::Draw(cv::InputOutputArray img, const cv::Ptr<Dict> &opts) const
{
    DrawTmplOptions dopts;
    if (opts)
    {
        dopts.colorTemplate     = opts->GetScalar(cv::String("TemplateColor"),      dopts.colorTemplate);
        dopts.colorRegion       = opts->GetScalar(cv::String("RegionColor"),        dopts.colorRegion);
        dopts.styleRegion       = opts->GetInt(cv::String("RegionStyle"),           dopts.styleRegion);
        dopts.colorArrow        = opts->GetScalar(cv::String("ArrowColor"),         dopts.colorArrow);
        dopts.thicknessArrow    = opts->GetReal32(cv::String("ArrowThickness"),     dopts.thicknessArrow);
        dopts.arrowSize         = opts->GetReal32(cv::String("ArrowSize"),          dopts.arrowSize);
        dopts.arrowTip          = opts->GetReal32(cv::String("ArrowTip"),           dopts.arrowTip);
        dopts.thicknessTemplate = opts->GetReal32(cv::String("TemplateThickness"),  dopts.thicknessTemplate);
        dopts.thicknessRegion   = opts->GetReal32(cv::String("RegionThickness"),    dopts.thicknessRegion);
    }

    for (int n = 0; n < MatchResultImpl::Count(); ++n)
    {
        if (contour_template_ && !contour_template_->Empty())
        {
            contour_template_->DrawInstance(img.getMat(), positions_[n], angles_[n], dopts);
        }

        if (pixel_template_)
        {
            pixel_template_->DrawInstance(img.getMat(), positions_[n], angles_[n], dopts);
        }
    }

    return MLR_SUCCESS;
}

int MatchResultImpl::DrawNth(cv::InputOutputArray img, const int nth, const cv::Ptr<Dict> &opts) const
{
    return MLR_SUCCESS;
}

}
}
