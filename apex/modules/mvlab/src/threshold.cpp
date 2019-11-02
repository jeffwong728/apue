#include "precomp.hpp"
#include "utility.hpp"
#include "region_impl.hpp"

namespace cv {
namespace mvlab {

class RunsPerRowCounter
{
public:
    RunsPerRowCounter(const cv::Mat *const imgMat, std::vector<int> *const numRunsPerRow, const uint32_t minGray, const uint32_t maxGray)
        : img_mat_(imgMat)
        , num_runs_per_row_(numRunsPerRow)
        , min_gray(minGray)
        , max_gray(maxGray)
    {}

    void operator()(const tbb::blocked_range<int>& br) const;

private:
    const cv::Mat *const img_mat_;
    std::vector<int> *const num_runs_per_row_;
    const uint32_t min_gray;
    const uint32_t max_gray;
};

void RunsPerRowCounter::operator()(const tbb::blocked_range<int>& br) const
{
    constexpr int32_t simdSize = 32;
    const int32_t width = img_mat_->cols;
    const auto stride = img_mat_->step1();
    const int32_t regularWidth = width & (-simdSize);
    const int32_t binWidth = regularWidth + simdSize;
    std::vector<int8_t, tbb::scalable_allocator<int8_t>> binVec(binWidth + 1);

    vcl::Vec32uc lowThresh(min_gray);
    vcl::Vec32uc highThresh(max_gray);

    uchar* pRow = img_mat_->data + br.begin() * stride;
    int *pNumRuns = num_runs_per_row_->data() + br.begin();

    for (int row = br.begin(); row < br.end(); ++row)
    {
        int col = 0;
        int8_t *pBin = binVec.data() + 1;
        const uchar* pCol = pRow;
        for (; col < regularWidth; col += simdSize)
        {
            vcl::Vec32uc pixelVals;
            pixelVals.load(pCol);

            vcl::Vec32c binVals = (pixelVals >= lowThresh) & (pixelVals <= highThresh);
            binVals.store(pBin);

            pBin += simdSize;
            pCol += simdSize;
        }

        if (col < width)
        {
            vcl::Vec32uc pixelVals;
            pixelVals.load_partial(width - col, pCol);

            vcl::Vec32c binVals = (pixelVals >= lowThresh) & (pixelVals <= highThresh);
            binVals.store_partial(width - col, pBin);
        }

        pBin = binVec.data();
        for (int n = 0; n < binWidth; n += simdSize)
        {
            vcl::Vec32c preVals, curVals;
            preVals.load(pBin);
            curVals.load(pBin + 1);
            *pNumRuns -= vcl::horizontal_add(preVals ^ curVals);

            pBin += simdSize;
        }

        *pNumRuns >>= 1;
        pRow += stride;
        pNumRuns += 1;
    }
}

class Thresholder
{
public:
    Thresholder(const cv::Mat *const imgMat, const std::vector<int> *const numRunsPerRow, RunList *const allRuns, const uint32_t minGray, const uint32_t maxGray)
        : img_mat_(imgMat)
        , num_runs_per_row_(numRunsPerRow)
        , all_runs_(allRuns)
        , min_gray(minGray)
        , max_gray(maxGray)
    {}

    void operator()(const tbb::blocked_range<int>& br) const;

private:
    const cv::Mat *const img_mat_;
    const std::vector<int> *const num_runs_per_row_;
    RunList *const all_runs_;
    const uint32_t min_gray;
    const uint32_t max_gray;
};

void Thresholder::operator()(const tbb::blocked_range<int>& br) const
{
    constexpr int32_t simdSize = 32;
    const int32_t width = img_mat_->cols;
    const auto stride = img_mat_->step1();
    const int32_t regularWidth = width & (-simdSize);
    const int32_t binWidth = regularWidth + simdSize;
    std::array<int8_t, simdSize> traVec;
    std::vector<int8_t, tbb::scalable_allocator<int8_t>> binVec(binWidth + 1);

    vcl::Vec32uc lowThresh(min_gray);
    vcl::Vec32uc highThresh(max_gray);

    uchar* pRow = img_mat_->data + br.begin() * stride;
    const int *pNumRuns = num_runs_per_row_->data() + br.begin();
    int preNumRuns = (br.begin() > 0) ? pNumRuns[-1] : 0;
    RunLength *pRun = all_runs_->data() + preNumRuns;

    for (int row = br.begin(); row < br.end(); ++row)
    {
        int col = 0;
        int8_t *pBin = binVec.data() + 1;
        const uchar* pCol = pRow;
        for (; col < regularWidth; col += simdSize)
        {
            vcl::Vec32uc pixelVals;
            pixelVals.load(pCol);

            vcl::Vec32c binVals = (pixelVals >= lowThresh) & (pixelVals <= highThresh);
            binVals.store(pBin);

            pBin += simdSize;
            pCol += simdSize;
        }

        if (col < width)
        {
            vcl::Vec32uc pixelVals;
            pixelVals.load_partial(width - col, pCol);

            vcl::Vec32c binVals = (pixelVals >= lowThresh) & (pixelVals <= highThresh);
            binVals.store_partial(width - col, pBin);
        }

        bool b = true;
        pBin = binVec.data();
        for (int n = 0; n < binWidth; n += simdSize)
        {
            vcl::Vec32c preVals, curVals;
            preVals.load(pBin);
            curVals.load(pBin + 1);
            vcl::Vec32c changeVals = preVals ^ curVals;
            if (vcl::horizontal_add(changeVals))
            {
                changeVals.store(traVec.data());
                for (int i = 0; i < simdSize; ++i)
                {
                    if (traVec[i])
                    {
                        if (b)
                        {
                            pRun->row = row;
                            pRun->colb = i + n;
                            b = false;
                        }
                        else
                        {
                            pRun->cole = i + n;
                            pRun += 1;
                            b = true;
                        }
                    }
                }
            }

            pBin += simdSize;
        }

        preNumRuns = *pNumRuns;
        pRow += stride;
        pNumRuns += 1;
    }
}

int Threshold(cv::InputArray src, const int minGray, const int maxGray, cv::Ptr<Region> &region)
{
    region = cv::makePtr<RegionImpl>();
    cv::Ptr<RegionImpl> rgnImpl = region.dynamicCast<RegionImpl>();
    if (!rgnImpl)
    {
        return MLR_MEMORY_ERROR;
    }

    cv::Mat imgMat = src.getMat();
    if (imgMat.empty())
    {
        return MLR_IMAGE_EMPTY;
    }

    if (minGray < 0 || minGray > 255)
    {
        return MLR_PARAMETER_ERROR + 1;
    }

    if (maxGray < 0 || maxGray > 255)
    {
        return MLR_PARAMETER_ERROR + 2;
    }

    if (minGray >= maxGray)
    {
        return MLR_PARAMETER_ERROR + 1;
    }

    int dph = imgMat.depth();
    int cnl = imgMat.channels();
    if (CV_8U != dph || 1 != cnl)
    {
        return MLR_IMAGE_FORMAT_ERROR;
    }

    std::vector<int> numRunsPerRow(imgMat.rows);
    RunsPerRowCounter numRunsCounter(&imgMat, &numRunsPerRow, minGray, maxGray);
    tbb::parallel_for(tbb::blocked_range<int>(0, imgMat.rows), numRunsCounter);

    for (int n = 1; n < imgMat.rows; ++n)
    {
        numRunsPerRow[n] += numRunsPerRow[n - 1];
    }

    rgnImpl->GetAllRuns().resize(numRunsPerRow.back());
    Thresholder thresher(&imgMat, &numRunsPerRow, &rgnImpl->GetAllRuns(), minGray, maxGray);
    tbb::parallel_for(tbb::blocked_range<int>(0, imgMat.rows), thresher);

    return MLR_SUCCESS;
}

}
}
