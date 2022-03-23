#include "precomp.hpp"
#include "mbox.h"
#include "utility.hpp"
#include <opencv2/mvlab/cmndef.hpp>
#include <opencv2/mvlab/contour.hpp>

namespace cv
{
namespace mvlab
{

cv::Ptr<MeasureBox> MeasureBox::Gen(const cv::RotatedRect &box, const cv::Size2f &sampleSize)
{
    if (sampleSize.width < 0.01f || sampleSize.height < 0.01f || box.size.width < 3.f || box.size.height < 0.f)
    {
        return nullptr;
    }

    return makePtr<MeasureBoxImpl>(box, sampleSize);
}

MeasureBoxImpl::MeasureBoxImpl(const cv::RotatedRect &box, const cv::Size2f &sampleSize)
    : box_(box), sample_size_(sampleSize)
{
    cv::Point2f points[4];
    box_.points(points);

    start_point_ = (points[0] + points[1]) / 2;
    end_point_ = (points[2] + points[3]) / 2;
    lengths_.width = box_.size.width;
    lengths_.height = box_.size.height;

    const float xt = sample_size_.width / lengths_.width;
    delta_1_ = xt * (end_point_ - start_point_);
    const float yt = sample_size_.height / lengths_.width;
    const cv::Point2f dPt = yt * (end_point_ - start_point_);
    delta_2_ = cv::Point2f(-dPt.y, dPt.x);
    SetSigma(1.f);
}

int MeasureBoxImpl::SetSigma(const float sigma) const
{
    const int kSize = Util::EstimateGaussianKernelSize(sigma);
    if (kSize >= 1)
    {
        cv::Mat gKernel = cv::getGaussianKernel(kSize, sigma, CV_32F);
        if (gKernel.rows == kSize && gKernel.cols == 1)
        {
            g_kernel_.resize(kSize);
            for (int r = 0; r < kSize; ++r)
            {
                g_kernel_[r] = gKernel.at<float>(r);
            }
            return MLR_SUCCESS;
        }
    }
    else
    {
        g_kernel_.resize(1);
        g_kernel_[0] = 1.f;
    }

    return MLR_ERROR;
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
    const int numSmooths = cvRound((lengths_.height / 2) / sample_size_.height);
    for (const cv::Point2f &lPt : lcenters)
    {
        for (int ss = 0; ss < numSmooths; ++ss)
        {
            centers.push_back(lPt + (ss + 1)*delta_2_);
            centers.push_back(lPt - (ss + 1)*delta_2_);
        }
    }

    centers.insert(centers.end(), lcenters.cbegin(), lcenters.cend());
    std::vector<cv::Size2f> sizes(centers.size(), sample_size_ / 3.f);
    std::vector<float> angles(centers.size(), box_.angle);

    return Contour::GenCross(centers, sizes, angles);
}

template <typename T>
int MeasureBoxImpl::GetProfileImpl(cv::InputArray img, std::vector<T> &grays) const
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
    const int numSmooths = cvRound((lengths_.height / 2) / sample_size_.height);
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
            grays.push_back(-10.);
        }
    }

    return MLR_SUCCESS;
}

int MeasureBoxImpl::GetProfile(cv::InputArray img, std::vector<double> &grays) const
{
    int iRes = GetProfileImpl<double>(img, grays);
    if (MLR_SUCCESS == iRes)
    {
        Smooth(grays);
    }
    return iRes;
}

void MeasureBoxImpl::Smooth(std::vector<double> &grays) const
{
    if (g_kernel_.size() > 1)
    {
        auto itValidBeg = std::find_if(grays.cbegin(), grays.cend(), [](const double v) { return v > -5; });
        auto itValidEnd = std::find_if(itValidBeg, grays.cend(), [](const double v) { return v < -5; });
        const auto cValidVals = std::distance(itValidBeg, itValidEnd);
        if (cValidVals > 1)
        {
            std::vector<double> bgrays(g_kernel_.size() - 1 + cValidVals);
            std::fill(bgrays.begin(), bgrays.begin() + g_kernel_.size() / 2, *itValidBeg);
            std::copy(itValidBeg, itValidEnd, bgrays.begin() + g_kernel_.size() / 2);
            std::fill(bgrays.begin() + g_kernel_.size() / 2 + cValidVals, bgrays.end(), *(itValidEnd - 1));
            grays.resize(cValidVals);
            for (int n = g_kernel_.size() / 2, m = 0; m < cValidVals; ++n, ++m)
            {
                grays[m] = 0;
                for (int i = n - g_kernel_.size() / 2, j=0; j < g_kernel_.size(); ++i, ++j)
                {
                    grays[m] += bgrays[i] * g_kernel_[j];
                }
            }
        }
    }
}

}
}
