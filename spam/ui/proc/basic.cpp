#include "basic.h"
#include <asmlib.h>
#include <memory>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>

std::vector<SpamRunListPool> BasicImgProc::s_runList_pools_;
std::vector<SpamRunList> BasicImgProc::s_rgn_pool_;

struct DarkThreshold
{
    DarkThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) const { return grayVal < upper; }
    const uchar lower;
    const uchar upper;
};

struct BrightThreshold
{
    BrightThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) const { return grayVal > lower; }
    const uchar lower;
    const uchar upper;
};

struct MundaneThreshold
{
    MundaneThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) const { return grayVal > lower && grayVal < upper; }
    const uchar lower;
    const uchar upper;
};

struct SIMDThreshold
{
    SIMDThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) const { return grayVal > lower; }
    const uchar lower;
    const uchar upper;
};

template <typename Pred>
class GeneralThresholdPI
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const Pred &pred, const int b, const int e, SpamRunListPool &pool)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e), runListPool_(pool) {  }

public:
    void operator()() const
    {
        constexpr int left = 0;
        const int right = grayImage_.cols;

        constexpr int blockSize = 1024*4;
        rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
        rgnList.back().resize(blockSize);
        auto pRuns = rgnList.back().data();
        int numRuns = 0;

        const auto stride = grayImage_.step1();
        const uchar* pRow = grayImage_.data + beg_ * stride;

        for (auto l = beg_; l != end_; ++l)
        {
            int cb = -1;
            const uchar* pPixel = pRow + left;
            for (int c = left; c < right; ++c) {
                if (pred_(*pPixel)) {
                    if (cb < 0) { cb = c; }
                } else {
                    if (cb > -1) {
                        if (numRuns == blockSize) {
                            numRuns = 0;
                            rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                            rgnList.back().resize(blockSize);
                            pRuns = rgnList.back().data();
                        }
                        pRuns[numRuns].l = l; pRuns[numRuns].cb = cb; pRuns[numRuns].ce = c; numRuns += 1; cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {
                if (numRuns == blockSize) {
                    rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                    numRuns = 0;
                    rgnList.back().resize(blockSize);
                    pRuns = rgnList.back().data();
                }

                pRuns[numRuns].l = l; pRuns[numRuns].cb = cb; pRuns[numRuns].ce = right; numRuns += 1;
            }
        }

        if (numRuns) {
            rgnList.back().resize(numRuns);
        } else {
            runListPool_.splice(runListPool_.end(), rgnList, --rgnList.cend());
        }
    }

public:
    mutable SpamRunListPool rgnList;
    const cv::Mat &grayImage_;
    const Pred &pred_;
    SpamRunListPool &runListPool_;
    const int beg_;
    const int end_;
};

template<>
struct GeneralThresholdPI<SIMDThreshold>
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const SIMDThreshold &pred, const int b, const int e, SpamRunListPool &pool)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e), runListPool_(pool) {  }

public:
    void operator()() const
    {
        const int width = grayImage_.cols;
        constexpr int blockSize = 1024 * 4;
        constexpr int simdSize = 32;
        rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
        rgnList.back().resize(blockSize);
        auto pRuns = rgnList.back().data();
        int numRuns = 0;

        bool sOk[simdSize] = {false};
        vcl::Vec32uc lowThresh(pred_.lower);
        const auto stride = grayImage_.step1();
        const uchar* pRow = grayImage_.data + beg_ * stride;
        const int regularWidth = width & (-simdSize);

        vcl::Vec32uc blockPixels;
        for (auto l = beg_; l != end_; ++l)
        {
            int c, cb = -1;
            const uchar* pPixel = pRow;
            for (c = 0; c < regularWidth; c += simdSize)
            {
                blockPixels.load(pPixel);
                pPixel += simdSize;
                vcl::Vec32cb r = blockPixels > lowThresh;
                auto numBright = vcl::horizontal_count(r);

                if (cb<0 && !numBright)
                {
                    continue;
                }

                if (cb>0 && simdSize == numBright)
                {
                    continue;
                }

                r.store(sOk);
                for (int i=0; i< simdSize; ++i)
                {
                    if (sOk[i]) {
                        if (cb < 0) { cb = c+i; }
                    } else {
                        if (cb > -1) {
                            if (numRuns == blockSize) {
                                numRuns = 0;
                                rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                                rgnList.back().resize(blockSize);
                                pRuns = rgnList.back().data();
                            }
                            pRuns[numRuns].l = l; pRuns[numRuns].cb = cb; pRuns[numRuns].ce = c+i; numRuns += 1; cb = -1;
                        }
                    }
                }
            }

            for (; c < width; ++c)
            {
                if (pred_(*pPixel)) {
                    if (cb < 0) { cb = c; }
                } else {
                    if (cb > -1) {
                        if (numRuns == blockSize) {
                            numRuns = 0;
                            rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                            rgnList.back().resize(blockSize);
                            pRuns = rgnList.back().data();
                        }
                        pRuns[numRuns].l = l; pRuns[numRuns].cb = cb; pRuns[numRuns].ce = c; numRuns += 1; cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {
                if (numRuns == blockSize) {
                    rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                    numRuns = 0;
                    rgnList.back().resize(blockSize);
                    pRuns = rgnList.back().data();
                }

                pRuns[numRuns].l = l; pRuns[numRuns].cb = cb; pRuns[numRuns].ce = width; numRuns += 1;
            }
        }

        if (numRuns) {
            rgnList.back().resize(numRuns);
        }
        else {
            runListPool_.splice(runListPool_.end(), rgnList, --rgnList.cend());
        }
    }

public:
    mutable SpamRunListPool rgnList;
    const cv::Mat &grayImage_;
    const SIMDThreshold &pred_;
    SpamRunListPool &runListPool_;
    const int beg_;
    const int end_;
};

template <typename Pred>
SPSpamRgn ThresholdPI_impl(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    SpamRunList &runList = rgn->GetData();

    if (CV_8U == dph && 1 == cnl)
    {
        Pred pred(lowerGray, upperGray);
        const int numTasks = std::min(10, static_cast<int>(BasicImgProc::s_runList_pools_.size()));
        const int rowStep = grayImage.rows / numTasks;
        boost::container::static_vector<GeneralThresholdPI<Pred>, 10> fObjs;
        for (int t = 0; t < numTasks - 1; ++t)
        {
            fObjs.emplace_back(grayImage, pred, rowStep*t, rowStep*(t + 1), BasicImgProc::s_runList_pools_[t]);
        }
        fObjs.emplace_back(grayImage, pred, rowStep*(numTasks - 1), grayImage.rows, BasicImgProc::s_runList_pools_[numTasks - 1]);

        switch (numTasks)
        {
        case 1: fObjs[0](); break;
        case 2: tbb::parallel_invoke(fObjs[0], fObjs[1]); break;
        case 4: tbb::parallel_invoke(fObjs[0], fObjs[1], fObjs[2], fObjs[3]); break;
        case 6: tbb::parallel_invoke(fObjs[0], fObjs[1], fObjs[2], fObjs[3], fObjs[4], fObjs[5]); break;
        case 8: tbb::parallel_invoke(fObjs[0], fObjs[1], fObjs[2], fObjs[3], fObjs[4], fObjs[5], fObjs[6], fObjs[7]); break;
        case 10: tbb::parallel_invoke(fObjs[0], fObjs[1], fObjs[2], fObjs[3], fObjs[4], fObjs[5], fObjs[6], fObjs[7], fObjs[8], fObjs[9]); break;
        default: break;
        }

        SpamRunListTBB::size_type numRuns = 0;
        for (const auto &fObj : fObjs)
        {
            for (const auto &rgnItem : fObj.rgnList)
            {
                numRuns += rgnItem.size();
            }
        }

        runList.swap(BasicImgProc::s_rgn_pool_.back());
        BasicImgProc::s_rgn_pool_.pop_back();
        runList.resize(numRuns);

        auto pDest = runList.data();
        for (int t = 0; t < numTasks; ++t)
        {
            for (const auto &rgnItem : fObjs[t].rgnList)
            {
                std::memcpy(pDest, rgnItem.data(), rgnItem.size() * sizeof(rgnItem.front()));
                pDest += rgnItem.size();
            }
            BasicImgProc::s_runList_pools_[t].splice(BasicImgProc::s_runList_pools_[t].end(), fObjs[t].rgnList);
        }
    }

    return rgn;
}

void BasicImgProc::Initialize(const int numWorkerThread)
{
    constexpr int cBlocks   = 30;
    constexpr int blockSize = 1024 * 4;
    s_runList_pools_.resize(numWorkerThread);
    for (auto &runListPool : s_runList_pools_)
    {
        runListPool.resize(cBlocks);
        for (auto &runList : runListPool)
        {
            runList.resize(blockSize);
        }
    }

    s_rgn_pool_.resize(5);
    for (auto &rgn : s_rgn_pool_)
    {
        rgn.resize(300000);
    }
}

void BasicImgProc::ReturnRegion(SpamRunList &&rgn)
{
    if (300000==rgn.capacity())
    {
        s_rgn_pool_.emplace_back(rgn);
    }
}

cv::Mat BasicImgProc::AlignImageWidth(const cv::Mat &img)
{
    constexpr int alignSize = 64;
    const auto stride = static_cast<int>(img.step1());
    const auto alignStride = stride & ~(alignSize - 1);
    if (alignStride == stride)
    {
        return img;
    }
    else
    {
        const auto alignWidth = (img.cols + alignSize - 1) & ~(alignSize - 1);
        cv::Mat alignedMat(img.rows, alignWidth, img.type());
        cv::Mat roiMat = alignedMat.colRange(0, img.cols);
        img.copyTo(roiMat);
        return roiMat;
    }
}

SPSpamRgn BasicImgProc::Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    if (0 == lowerGray)
    {
        return ThresholdPI_impl<DarkThreshold>(grayImage, lowerGray, upperGray);
    }
    else if (255 == upperGray)
    {
        return ThresholdPI_impl<SIMDThreshold>(grayImage, lowerGray, upperGray);
    }
    else
    {
        return ThresholdPI_impl<MundaneThreshold>(grayImage, lowerGray, upperGray);
    }
}

SPSpamRgnVector BasicImgProc::Connect(const cv::Mat &labels, const int numLabels)
{
    SPSpamRgnVector rgs = std::make_shared<SpamRgnVector>();
    rgs->resize(numLabels);
    SpamRgnVector &rgns = *rgs;

    const int top = 0;
    const int bot = labels.rows;
    const int left = 0;
    const int right = labels.cols;
    const auto stride = labels.step1();
    for (int r = top; r < bot; ++r)
    {
        int cb = -1;
        const int* pRow = reinterpret_cast<const int *>(labels.data + r * stride);
        for (int c = left; c < right; ++c)
        {
            if (pRow[c])
            {
                if (cb < 0)
                {
                    cb = c;
                }
            }
            else
            {
                if (cb > -1)
                {
                    rgns[pRow[cb]-1].data_.push_back({ r, cb, c });
                    cb = -1;
                }
            }
        }

        if (cb > -1)
        {
            rgns[pRow[cb]-1].data_.push_back({ r, cb, right });
        }
    }

    return SPSpamRgnVector();
}