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
    GeneralThresholdPI(const cv::Mat &grayImage, const Pred &pred, const int16_t b, const int16_t e, SpamRunListPool &pool)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e), runListPool_(pool) {  }

public:
    void operator()() const
    {
        constexpr int16_t left = 0;
        const int16_t right = static_cast<int16_t>(grayImage_.cols);

        constexpr int blockSize = 1024*4;
        rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
        rgnList.back().resize(blockSize);
        auto pRuns = rgnList.back().data();
        int numRuns = 0;

        const auto stride = grayImage_.step1();
        const uchar* pRow = grayImage_.data + beg_ * stride;

        for (auto l = beg_; l != end_; ++l)
        {
            int16_t cb = -1;
            const uchar* pPixel = pRow + left;
            for (int16_t c = left; c < right; ++c) {
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
                        pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = c; numRuns += 1; cb = -1;
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

                pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = right; numRuns += 1;
            }
        }

        if (numRuns) {
            rgnList.back().resize(numRuns);
        } else {
            runListPool_.splice(runListPool_.end(), rgnList, --rgnList.cend());
        }
    }

public:
    SpamRunListTBB rgn_;
    const cv::Mat &grayImage_;
    const Pred &pred_;
    const int16_t beg_;
    const int16_t end_;
};

template<>
struct GeneralThresholdPI<SIMDThreshold>
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const SIMDThreshold &pred, const int16_t b, const int16_t e, SpamRunListPool &pool)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e), runListPool_(pool) {  }

public:
    void operator()() const
    {
        const int16_t width = static_cast<int16_t>(grayImage_.cols);
        constexpr int blockSize = 1024 * 4;
        constexpr int16_t simdSize = 32;
        rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
        rgnList.back().resize(blockSize);
        auto pRuns = rgnList.back().data();
        int numRuns = 0;

        bool sOk[simdSize] = {false};
        vcl::Vec32uc lowThresh(pred_.lower);
        const auto stride = grayImage_.step1();
        const uchar* pRow = grayImage_.data + beg_ * stride;
        const int16_t regularWidth = width & (-simdSize);

        vcl::Vec32uc blockPixels;
        for (auto l = beg_; l != end_; ++l)
        {
            int16_t c, cb = -1;
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
                for (int16_t i=0; i< simdSize; ++i)
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
                            pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = c+i; numRuns += 1; cb = -1;
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
                        pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = c; numRuns += 1; cb = -1;
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

                pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = width; numRuns += 1;
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
    const int16_t beg_;
    const int16_t end_;
};

template <typename Pred>
class ThresholdTBBReducer
{
public:
    ThresholdTBBReducer(const cv::Mat &grayImage, const Pred &pred, SpamRun *beg, const int cap)
        : grayImage_(grayImage), pred_(pred), beg_(beg), capacity_(cap), size_(0) {  }

    ThresholdTBBReducer(ThresholdTBBReducer& x, tbb::split) 
        : grayImage_(x.grayImage_)
        , pred_(x.pred_)
        , beg_(x.beg_ + x.capacity_/2)
        , capacity_(x.capacity_ - x.capacity_ / 2)
        , size_(0)
    {
        x.capacity_ = x.capacity_ / 2;
    }

    void join(const ThresholdTBBReducer& y) 
    { 
        const int remainSpace = capacity_ - size_;
        if (remainSpace<y.size_)
        {
            ::A_memmove(beg_+ size_, y.beg_, y.size_*sizeof(SpamRun));
        }
        else
        {
            ::A_memcpy(beg_ + size_, y.beg_, y.size_ * sizeof(SpamRun));
        }

        capacity_ += y.capacity_;
        size_ += y.size_;
    }

    void operator()(const tbb::blocked_range<int16_t> &r)
    {
        constexpr int16_t left = 0;
        const int16_t right = static_cast<int16_t>(grayImage_.cols);

        auto pRuns = beg_ + size_;
        int numRuns = 0;

        const auto stride = grayImage_.step1();
        const uchar* pRow = grayImage_.data + r.begin() * stride;

        const auto end = r.end();
        for (auto l = r.begin(); l != end; ++l)
        {
            int16_t cb = -1;
            const uchar* pPixel = pRow + left;
            for (int16_t c = left; c < right; ++c) {
                if (pred_(*pPixel)) {
                    if (cb < 0) { cb = c; }
                }
                else {
                    if (cb > -1) {
                        pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = c; numRuns += 1; cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {
                pRuns[numRuns].row = l; pRuns[numRuns].colb = cb; pRuns[numRuns].cole = right; numRuns += 1;
            }
        }

        size_ += numRuns;
    }

public:
    const cv::Mat &grayImage_;
    const Pred &pred_;
    SpamRun *beg_;
    int capacity_;
    int size_;
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

        runList.resize(numRuns);
        auto pDest = runList.data();
        for (int t = 0; t < numTasks; ++t)
        {
            for (const auto &rgnItem : fObjs[t].rgnList)
            {
                ::A_memcpy(pDest, rgnItem.data(), rgnItem.size() * sizeof(rgnItem.front()));
                pDest += rgnItem.size();
            }
            BasicImgProc::s_runList_pools_[t].splice(BasicImgProc::s_runList_pools_[t].end(), fObjs[t].rgnList);
        }
    }

    return rgn;
}

template <typename Pred>
SPSpamRgn ThresholdTBBReduce_impl(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    SpamRunList &runList = rgn->GetData();

    if (CV_8U == dph && 1 == cnl)
    {
        runList.swap(BasicImgProc::s_rgn_pool_.back());
        BasicImgProc::s_rgn_pool_.pop_back();
        runList.resize(1024*1024);

        Pred pred(lowerGray, upperGray);
        ThresholdTBBReducer reducer(grayImage, pred, runList.data(), static_cast<int>(runList.capacity()));
        const int rowStep = grayImage.rows / static_cast<int>(BasicImgProc::s_runList_pools_.size());
        tbb::parallel_deterministic_reduce(tbb::blocked_range<int16_t>(0, static_cast<int16_t>(grayImage.rows), rowStep), reducer);
        runList.resize(reducer.size_);
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
        rgn.resize(1024*1024);
    }
}

void BasicImgProc::ReturnRegion(SpamRunList &&rgn)
{
    if (1024*1024==rgn.capacity())
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

    const int16_t top = 0;
    const int16_t bot = static_cast<int16_t>(labels.rows);
    const int16_t left = 0;
    const int16_t right = static_cast<int16_t>(labels.cols);
    const auto stride = labels.step1();
    for (int16_t r = top; r < bot; ++r)
    {
        int16_t cb = -1;
        const int* pRow = reinterpret_cast<const int *>(labels.data + r * stride);
        for (int16_t c = left; c < right; ++c)
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