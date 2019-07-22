#include "basic.h"
#include <memory>
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

template <typename Pred>
SPSpamRgn GeneralThreshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    Pred pred(lowerGray, upperGray);
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    rgn->data_.reserve(grayImage.rows*grayImage.cols);
    if (CV_8U == dph && 1 == cnl)
    {
        int top = 0;
        int bot = grayImage.rows;
        int left = 0;
        int right = grayImage.cols;
        for (int r = top; r<bot; ++r)
        {
            int cb = -1;
            const uchar* pRow = grayImage.data + r * grayImage.step1();
            for (int c = left; c < right; ++c)
            {
                if (pred(pRow[c]))
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
                        rgn->data_.push_back({ r, cb, c });
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                rgn->data_.push_back({ r, cb, right });
            }
        }
    }

    return rgn;
}

template <typename Pred>
class GeneralThresholdPI
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const Pred &pred, const int b, const int e, SpamRunListPool &pool)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e), runListPool_(pool) {  }

public:
    void operator()() const
    {
        int left = 0;
        int right = grayImage_.cols;

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
            for (int c = left; c < right; ++c)
            {
                if (pred_(pRow[c]))
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
                        if (numRuns == blockSize)
                        {
                            rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                            numRuns = 0;
                            rgnList.back().resize(blockSize);
                            pRuns = rgnList.back().data();
                        }

                        pRuns[numRuns].l = l;
                        pRuns[numRuns].cb = cb;
                        pRuns[numRuns].ce = c;
                        numRuns += 1;
                        cb = -1;
                    }
                }
            }

            pRow += stride;

            if (cb > -1)
            {
                if (numRuns == blockSize)
                {
                    rgnList.splice(rgnList.end(), runListPool_, runListPool_.cbegin());
                    numRuns = 0;
                    rgnList.back().resize(blockSize);
                    pRuns = rgnList.back().data();
                }

                pRuns[numRuns].l = l;
                pRuns[numRuns].cb = cb;
                pRuns[numRuns].ce = right;
                numRuns += 1;
            }
        }

        if (numRuns)
        {
            rgnList.back().resize(numRuns);
        }
        else
        {
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

SPSpamRgn BasicImgProc::Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    SpamRunList &runList = rgn->GetData();

    if (CV_8U == dph && 1 == cnl)
    {
        if (0 == lowerGray)
        {
            DarkThreshold pred(lowerGray, upperGray);
            const int rowStep = grayImage.rows / 4;
            GeneralThresholdPI f0(grayImage, pred, 0, rowStep, s_runList_pools_[0]);
            GeneralThresholdPI f1(grayImage, pred, rowStep, rowStep * 2, s_runList_pools_[1]);
            GeneralThresholdPI f2(grayImage, pred, rowStep*2, rowStep * 3, s_runList_pools_[2]);
            GeneralThresholdPI f3(grayImage, pred, rowStep*3, rowStep * 4, s_runList_pools_[3]);

            tbb::parallel_invoke(f0, f1, f2, f3);

            SpamRunListPool *rgnLists[]{ &f0.rgnList , &f1.rgnList, &f2.rgnList, &f3.rgnList};

            SpamRunListTBB::size_type numRuns = 0;
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    numRuns += rgnItem.size();
                }
            }

            runList.swap(s_rgn_pool_.back());
            s_rgn_pool_.pop_back();
            runList.resize(numRuns);

            auto pDest = runList.data();
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    std::memcpy(pDest, rgnItem.data(), rgnItem.size() * sizeof(rgnItem.front()));
                    pDest += rgnItem.size();
                }
            }

            s_runList_pools_[0].splice(s_runList_pools_[0].end(), f0.rgnList);
            s_runList_pools_[1].splice(s_runList_pools_[1].end(), f1.rgnList);
            s_runList_pools_[2].splice(s_runList_pools_[2].end(), f2.rgnList);
            s_runList_pools_[3].splice(s_runList_pools_[3].end(), f3.rgnList);
        }
        else if (255 == upperGray)
        {
            BrightThreshold pred(lowerGray, upperGray);
            const int rowStep = grayImage.rows / 4;
            GeneralThresholdPI f0(grayImage, pred, 0, rowStep, s_runList_pools_[0]);
            GeneralThresholdPI f1(grayImage, pred, rowStep, rowStep * 2, s_runList_pools_[1]);
            GeneralThresholdPI f2(grayImage, pred, rowStep * 2, rowStep * 3, s_runList_pools_[2]);
            GeneralThresholdPI f3(grayImage, pred, rowStep * 3, rowStep * 4, s_runList_pools_[3]);

            tbb::parallel_invoke(f0, f1, f2, f3);
            SpamRunListPool *rgnLists[]{ &f0.rgnList , &f1.rgnList, &f2.rgnList, &f3.rgnList};

            SpamRunListTBB::size_type numRuns = 0;
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    numRuns += rgnItem.size();
                }
            }

            runList.swap(s_rgn_pool_.back());
            s_rgn_pool_.pop_back();
            runList.resize(numRuns);

            auto pDest = runList.data();
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    std::memcpy(pDest, rgnItem.data(), rgnItem.size() * sizeof(rgnItem.front()));
                    pDest += rgnItem.size();
                }
            }

            s_runList_pools_[0].splice(s_runList_pools_[0].end(), f0.rgnList);
            s_runList_pools_[1].splice(s_runList_pools_[1].end(), f1.rgnList);
            s_runList_pools_[2].splice(s_runList_pools_[2].end(), f2.rgnList);
            s_runList_pools_[3].splice(s_runList_pools_[3].end(), f3.rgnList);
        }
        else
        {
            MundaneThreshold pred(lowerGray, upperGray);
            const int rowStep = grayImage.rows / 4;
            GeneralThresholdPI f0(grayImage, pred, 0, rowStep, s_runList_pools_[0]);
            GeneralThresholdPI f1(grayImage, pred, rowStep, rowStep * 2, s_runList_pools_[1]);
            GeneralThresholdPI f2(grayImage, pred, rowStep * 2, rowStep * 3, s_runList_pools_[2]);
            GeneralThresholdPI f3(grayImage, pred, rowStep * 3, rowStep * 4, s_runList_pools_[3]);

            tbb::parallel_invoke(f0, f1, f2, f3);
            SpamRunListPool *rgnLists[]{ &f0.rgnList , &f1.rgnList, &f2.rgnList, &f3.rgnList };

            SpamRunListTBB::size_type numRuns = 0;
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    numRuns += rgnItem.size();
                }
            }

            runList.swap(s_rgn_pool_.back());
            s_rgn_pool_.pop_back();
            runList.resize(numRuns);

            auto pDest = runList.data();
            for (const auto &rgnList : rgnLists)
            {
                for (const auto &rgnItem : *rgnList)
                {
                    std::memcpy(pDest, rgnItem.data(), rgnItem.size() * sizeof(rgnItem.front()));
                    pDest += rgnItem.size();
                }
            }

            s_runList_pools_[0].splice(s_runList_pools_[0].end(), f0.rgnList);
            s_runList_pools_[1].splice(s_runList_pools_[1].end(), f1.rgnList);
            s_runList_pools_[2].splice(s_runList_pools_[2].end(), f2.rgnList);
            s_runList_pools_[3].splice(s_runList_pools_[3].end(), f3.rgnList);
        }
    }

    return rgn;
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
                    //rgns[pRow[cb]-1].data_.push_back({ r, cb, c });
                    cb = -1;
                }
            }
        }

        if (cb > -1)
        {
            //rgns[pRow[cb]-1].data_.push_back({ r, cb, right });
        }
    }

    return SPSpamRgnVector();
}