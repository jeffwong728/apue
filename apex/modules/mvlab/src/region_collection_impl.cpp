#include "precomp.hpp"
#include "region_collection_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

int RegionCollectionImpl::Draw(Mat &img, const Scalar& color, const float thickness, const int style) const
{
    return MLR_SUCCESS;
}

int RegionCollectionImpl::Draw(InputOutputArray img, const Scalar& color, const float thickness, const int style) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int res = RegionCollectionImpl::Draw(imgMat, color, thickness, style);
        img.assign(imgMat);
        return res;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = RegionCollectionImpl::Draw(imgMat, color, thickness, style);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

void RegionCollectionImpl::Area(std::vector<double> &areas) const
{
    areas.resize(0);
}

void RegionCollectionImpl::Length(std::vector<double> &lengths) const
{
    lengths.resize(0);
}

void RegionCollectionImpl::Centroid(std::vector<cv::Point2d> &centroids) const
{
    centroids.resize(0);
}

void RegionCollectionImpl::BoundingBox(std::vector<cv::Rect> &bboxs) const
{
    bboxs.resize(0);
}

void RegionCollectionImpl::ClearCacheData()
{
}

void RegionCollectionImpl::DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const
{
}

}
}
