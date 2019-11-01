#ifndef __OPENCV_MVLAB_REGION_COLLECTION_IMPL_HPP__
#define __OPENCV_MVLAB_REGION_COLLECTION_IMPL_HPP__

#include "region_impl.hpp"
#include <opencv2/mvlab/region_collection.hpp>

namespace cv {
namespace mvlab {

class RegionCollectionImpl : public RegionCollection
{
public:
    RegionCollectionImpl() {}

public:
    int Draw(Mat &img, const Scalar& color, const float thickness, const int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const Scalar& color, float thickness, const int style) const CV_OVERRIDE;

public:
    void Area(std::vector<double> &areas) const CV_OVERRIDE;
    void Length(std::vector<double> &lengths) const CV_OVERRIDE;
    void Centroid(std::vector<cv::Point2d> &centroids) const CV_OVERRIDE;
    void BoundingBox(std::vector<cv::Rect> &bboxs) const CV_OVERRIDE;

private:
    void ClearCacheData();
    void DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const;

private:
    RunList all_rgn_runs_;
};

}
}

#endif //__OPENCV_MVLAB_REGION_COLLECTION_IMPL_HPP__
