#include "precomp.hpp"
#include "region_collection_impl.hpp"
#include "utility.hpp"
#include <opencv2/mvlab.hpp>

namespace cv {
namespace mvlab {

int RegionCollectionImpl::Draw(Mat &img, const std::vector<cv::Scalar> &colors, const float thickness, const int style) const
{
    return MLR_SUCCESS;
}

int RegionCollectionImpl::Draw(InputOutputArray img, const std::vector<cv::Scalar> &colors, const float thickness, const int style) const
{
    Mat imgMat = img.getMat();
    if (imgMat.empty())
    {
        int res = RegionCollectionImpl::Draw(imgMat, colors, thickness, style);
        img.assign(imgMat);
        return res;
    }
    else
    {
        int dph = img.depth();
        int cnl = img.channels();
        if (CV_8U == dph && (1 == cnl || 3 == cnl || 4 == cnl))
        {
            int rest = RegionCollectionImpl::Draw(imgMat, colors, thickness, style);
            img.assign(imgMat);
            return rest;
        }
        else
        {
            return MLR_IMAGE_FORMAT_ERROR;
        }
    }
}

int RegionCollectionImpl::Count() const
{
    return static_cast<int>(run_beg_idxs_.size());
}

void RegionCollectionImpl::Area(std::vector<double> &areas) const
{
    GatherBasicFeatures();
    areas.assign(rgn_areas_.cbegin(), rgn_areas_.cend());
}

void RegionCollectionImpl::Length(std::vector<double> &lengths) const
{
    lengths.resize(0);
}

void RegionCollectionImpl::Centroid(std::vector<cv::Point2d> &centroids) const
{
    centroids.assign(rgn_centroids_.cbegin(), rgn_centroids_.cend());
}

void RegionCollectionImpl::BoundingBox(std::vector<cv::Rect> &bboxs) const
{
    bboxs.assign(rgn_bboxes_.cbegin(), rgn_bboxes_.cend());
}

void RegionCollectionImpl::DrawVerified(Mat &img, const Scalar& color, const float thickness, const int style) const
{
}

void RegionCollectionImpl::GatherBasicFeatures() const
{
    const int numRgns = RegionCollectionImpl::Count();
    if (!numRgns || !rgn_areas_.empty())
    {
        return;
    }

    rgn_row_run_begs_.resize(numRgns);
    rgn_areas_.resize(numRgns);
    rgn_centroids_.resize(numRgns);
    rgn_bboxes_.resize(numRgns);

    double *pAreas = rgn_areas_.data();
    cv::Point2d *pCentroids = rgn_centroids_.data();
    cv::Rect *pBBoxes = rgn_bboxes_.data();
    RowRunStartList *pRowRunStarts = rgn_row_run_begs_.data();

    int rgnIdxBeg = 0;
    const int *pRgnIdxEnd = run_beg_idxs_.data() + run_beg_idxs_.size();
    const RunLength *rgnRunBeg = all_rgn_runs_.data();
    for (const int *pRgnIdx = run_beg_idxs_.data(); pRgnIdx != pRgnIdxEnd; ++pRgnIdx)
    {
        const RunLength *rgnRunEnd = all_rgn_runs_.data() + *pRgnIdx;
        pRowRunStarts->reserve(rgnRunEnd[-1].row - rgnRunBeg->row + 2);

        double a = 0, x = 0, y = 0;
        cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
        cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

        int begIdx = rgnIdxBeg, runIdx = rgnIdxBeg;
        int currentRow = rgnRunBeg->row;
        for (const RunLength *pRun = rgnRunBeg; pRun != rgnRunEnd; ++pRun, ++runIdx)
        {
            const RunLength &rl = *pRun;
            if (rl.row != currentRow)
            {
                pRowRunStarts->emplace_back(begIdx); begIdx = runIdx; currentRow = rl.row;
            }

            const auto n = rl.cole - rl.colb;
            a += n;
            x += (rl.cole - 1 + rl.colb) * n / 2.0;
            y += rl.row * n;

            if (rl.row < minPoint.y) { minPoint.y = rl.row; }
            if (rl.row > maxPoint.y) { maxPoint.y = rl.row; }
            if (rl.colb < minPoint.x) { minPoint.x = rl.colb; }
            if (rl.cole > maxPoint.x) { maxPoint.x = rl.cole; }
        }

        pRowRunStarts->emplace_back(begIdx);
        pRowRunStarts->emplace_back(runIdx);

        *pAreas = a;
        if (a > 0) {
            pCentroids->x = x / a; pCentroids->y = y / a;
        }
        else { 
            pCentroids->x = 0; pCentroids->y = 0;
        }

        *pBBoxes = cv::Rect(minPoint, maxPoint);

        pAreas += 1;
        pCentroids += 1;
        pBBoxes += 1;
        pRowRunStarts += 1;
        rgnRunBeg = rgnRunEnd;
        rgnIdxBeg = *pRgnIdx;
    }
}

}
}
