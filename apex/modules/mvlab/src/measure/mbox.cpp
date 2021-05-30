#include "precomp.hpp"
#include "mbox.h"
#include "utility.hpp"
#include <opencv2/mvlab/cmndef.hpp>
#include <opencv2/mvlab/contour.hpp>

namespace cv
{
namespace mvlab
{

cv::Ptr<MeasureBox> MeasureBox::GenMeasureBox(const cv::RotatedRect &box, const cv::Point2f &sampleSize)
{
    if (sampleSize.x < 0.01f || sampleSize.y < 0.01f || box.size.width < 3.f || box.size.height < 0.f)
    {
        return nullptr;
    }

    return makePtr<MeasureBoxImpl>(box, sampleSize);
}

MeasureBoxImpl::MeasureBoxImpl(const cv::RotatedRect &box, const cv::Point2f &sampleSize)
    : box_(box), sample_size_(sampleSize)
{
    cv::Point2f points[4];
    box_.points(points);

    start_point_ = (points[0] + points[1]) / 2;
    end_point_ = (points[2] + points[3]) / 2;
    lengths_.x = box_.size.width;
    lengths_.y = box_.size.height;

    const float xt = sample_size_.x / lengths_.x;
    delta_1_ = xt * (end_point_ - start_point_);
    const float yt = sample_size_.y / lengths_.x;
    const cv::Point2f dPt = yt * (end_point_ - start_point_);
    delta_2_ = cv::Point2f(-dPt.y, dPt.x);
}

cv::Ptr<Contour> MeasureBoxImpl::GetMarks() const
{
    if (!Valid())
    {
        return Contour::GenEmpty();
    }

    std::vector<cv::Point2f> lcenters;
    lcenters.push_back(start_point_);
    for (int ll=0; ll < NumLines(); ++ll)
    {
        lcenters.push_back(start_point_ + (ll + 1) * delta_1_);
    }

    if (lcenters.back().ddot(end_point_) > 0.25)
    {
        lcenters.push_back(end_point_);
    }

    std::vector<cv::Point2f> centers;
    const int numSmooths = cvRound((lengths_.y / 2) / sample_size_.y);
    for (const cv::Point2f &lPt : lcenters)
    {
        for (int ss = 0; ss < numSmooths; ++ss)
        {
            centers.push_back(lPt + (ss + 1)*delta_2_);
            centers.push_back(lPt - (ss + 1)*delta_2_);
        }
    }

    centers.insert(centers.end(), lcenters.cbegin(), lcenters.cend());
    std::vector<cv::Size2f> sizes(centers.size(), sample_size_ / 3);
    std::vector<float> angles(centers.size(), box_.angle);

    return Contour::GenCross(centers, sizes, angles);
}

int MeasureBoxImpl::GetProfile(cv::InputArray img, std::vector<double> &grays) const
{
    grays.resize(0);
    if (!Valid())
    {
        return MLR_MEASURE_BOX_INVALID;
    }

    cv::Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    int dph = imgMat.depth();
    int cnl = imgMat.channels();
    if (CV_8U != dph || 1 != cnl)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    grays.reserve(NumLines()+1);
    const int numSmooths = cvRound((lengths_.y / 2) / sample_size_.y);
    for (int ll = 0; ll <= NumLines(); ++ll)
    {
        int numValidPoints = 0;
        float graySum = 0.;
        const cv::Point2f lCenter = start_point_ + ll * delta_1_;
        float grayPt = Util::InterpolateBiLinear(imgMat, lCenter);
        if (grayPt >= 0.f)
        {
            graySum += grayPt;
            numValidPoints += 1;
        }

        for (int ss = 0; ss < numSmooths; ++ss)
        {
            const cv::Point2f ptPos = lCenter + (ss + 1) * delta_2_;
            const cv::Point2f ptNeg = lCenter - (ss + 1) * delta_2_;

            grayPt = Util::InterpolateBiLinear(imgMat, ptPos);
            if (grayPt >= 0.f)
            {
                graySum += grayPt;
                numValidPoints += 1;
            }

            grayPt = Util::InterpolateBiLinear(imgMat, ptNeg);
            if (grayPt >= 0.f)
            {
                graySum += grayPt;
                numValidPoints += 1;
            }
        }

        if (numValidPoints)
        {
            grays.push_back(graySum / numValidPoints);
        }
        else
        {
            grays.push_back(-1.0);
        }
    }

    return MLR_SUCCESS;
}

}
}
