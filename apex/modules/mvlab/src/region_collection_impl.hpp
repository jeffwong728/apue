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
    RegionCollectionImpl(RunList *runs, std::vector<int> *begIdxs) : all_rgn_runs_(std::move(*runs)), run_beg_idxs_(std::move(*begIdxs)) {}

public:
    int Draw(Mat &img, const std::vector<cv::Scalar> &colors, const float thickness, const int style) const CV_OVERRIDE;
    int Draw(InputOutputArray img, const std::vector<cv::Scalar> &colors, float thickness, const int style) const CV_OVERRIDE;

public:
    int Count() const CV_OVERRIDE;
    void Area(std::vector<double> &areas) const CV_OVERRIDE;
    void Length(std::vector<double> &lengths) const CV_OVERRIDE;
    void Centroid(std::vector<cv::Point2d> &centroids) const CV_OVERRIDE;
    void BoundingBox(std::vector<cv::Rect> &bboxs) const CV_OVERRIDE;

private:
    void DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const;
    void GatherBasicFeatures() const;

private:
    const RunList all_rgn_runs_;
    const std::vector<int> run_beg_idxs_;

    mutable std::vector<RowRunStartList>            rgn_row_run_begs_;
    mutable std::vector<double>                     rgn_areas_;
    mutable std::vector<cv::Point2d>                rgn_centroids_;
    mutable std::vector<cv::Rect>                   rgn_bboxes_;
    mutable std::vector<std::vector<Ptr<Contour>>>  rgn_contour_outers_;
    mutable std::vector<std::vector<Ptr<Contour>>>  rgn_contour_holes_;
};

}
}

#endif //__OPENCV_MVLAB_REGION_COLLECTION_IMPL_HPP__
