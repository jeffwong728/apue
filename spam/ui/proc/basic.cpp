#include "basic.h"
#include <memory>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

using SpamRunListTBB = std::vector<SpamRun, tbb::scalable_allocator<SpamRun>>;
using UPSpamRunListTBB = std::unique_ptr<SpamRunListTBB>;
using SpamRunListList = std::vector<std::unique_ptr<SpamRunListTBB>, tbb::scalable_allocator<std::unique_ptr<SpamRunListTBB>>>;

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
class GeneralThresholdTBB
{
public:
    GeneralThresholdTBB(const cv::Mat &grayImage, const Pred &pred) : grayImage_(grayImage), pred_(pred) { rgnList.reserve(grayImage.rows); }
    GeneralThresholdTBB(GeneralThresholdTBB& x, tbb::split) : grayImage_(x.grayImage_), pred_(x.pred_) {}
    void join(GeneralThresholdTBB& y)
    {
        rgnList.insert(rgnList.end(), std::make_move_iterator(y.rgnList.begin()), std::make_move_iterator(y.rgnList.end()));
    }

public:
    void operator()(const tbb::blocked_range<int> &r)
    {
        int left = 0;
        int right = grayImage_.cols;

        std::unique_ptr<SpamRunListTBB> rgn = std::make_unique<SpamRunListTBB>();
        for (auto l = r.begin(); l != r.end(); ++l)
        {
            int cb = -1;
            const uchar* pRow = grayImage_.data + l * grayImage_.step1();
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
                        rgn->emplace_back(l, cb, c);
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                rgn->emplace_back(l, cb, right);
            }
        }

        if (!rgn->empty())
        {
            rgnList.push_back(std::move(rgn));
        }
    }

public:
    SpamRunListList rgnList;
    const cv::Mat &grayImage_;
    const Pred &pred_;
};

template <typename Pred>
class GeneralThreshold2
{
public:
    GeneralThreshold2(const cv::Mat &grayImage, const Pred &pred, std::vector<PackRun> &allRuns, std::vector<int> &numRuns)
        : grayImage_(grayImage), pred_(pred), allRuns_(allRuns), numRuns_(numRuns) { }

public:
    void operator()(const tbb::blocked_range<int> &r) const
    {
        int left = 0;
        int right = grayImage_.cols;

        auto imgStep  = grayImage_.step1();
        auto pRuns    = allRuns_.data() + r.begin() * grayImage_.cols;
        auto pNumRuns = numRuns_.data() + r.begin();
        const uchar* pRow = grayImage_.data + r.begin() * imgStep;

        for (auto l = r.begin(); l != r.end(); ++l)
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
                        pRuns[*pNumRuns].b = cb;
                        pRuns[*pNumRuns].e = c;
                        *pNumRuns += 1;
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                pRuns[*pNumRuns].b = cb;
                pRuns[*pNumRuns].e = right;
                *pNumRuns += 1;
            }

            pRuns += grayImage_.cols;
            pNumRuns += 1;
            pRow += imgStep;
        }
    }

public:
    const cv::Mat &grayImage_;
    const Pred &pred_;
    std::vector<PackRun> &allRuns_;
    std::vector<int>     &numRuns_;
};

SPSpamRgn BasicImgProc::Threshold2(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    SpamRunList &runList = rgn->GetData();

    std::vector<PackRun> allRuns(grayImage.rows * grayImage.cols);
    std::vector<int>     numRuns(grayImage.rows);

    if (CV_8U == dph && 1 == cnl)
    {
        std::memset(numRuns.data(), 0, sizeof(numRuns.front())*grayImage.rows);
        if (0 == lowerGray)
        {
            DarkThreshold pred(lowerGray, upperGray);
            GeneralThreshold2<DarkThreshold> thresholder(grayImage, pred, allRuns, numRuns);

            tbb::parallel_for(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
        }
        else if (255 == upperGray)
        {
            BrightThreshold pred(lowerGray, upperGray);
            GeneralThreshold2<BrightThreshold> thresholder(grayImage, pred, allRuns, numRuns);

            tbb::parallel_for(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
        }
        else
        {
            MundaneThreshold pred(lowerGray, upperGray);
            GeneralThreshold2<MundaneThreshold> thresholder(grayImage, pred, allRuns, numRuns);

            tbb::parallel_for(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
        }

        int tNumRuns = 0;
        for (const auto &n : numRuns)
        {
            tNumRuns += n;
        }

        auto pRuns = allRuns.data();
        auto pNumRuns = numRuns.data();

        runList.resize(tNumRuns);
        auto pDest = runList.data();
        for (int row=0; row<grayImage.rows; ++row)
        {
            for (int r=0; r<*pNumRuns; ++r)
            {
                pDest->l = row;
                pDest->cb = pRuns[r].b;
                pDest->ce = pRuns[r].e;
                pDest += 1;
            }

            pRuns += grayImage.cols;
            pNumRuns += 1;
        }
    }

    return rgn;
}

SPSpamRgn BasicImgProc::Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    int dph = grayImage.depth();
    int cnl = grayImage.channels();
    SPSpamRgn rgn = std::make_shared<SpamRgn>();
    SpamRunList &runList = rgn->GetData();

    if (CV_8U == dph && 1 == cnl)
    {
        SpamRunListList rgnList;
        if (0 == lowerGray)
        {
            DarkThreshold pred(lowerGray, upperGray);
            GeneralThresholdTBB<DarkThreshold> thresholder(grayImage, pred);

            tbb::parallel_reduce(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
            rgnList.swap(thresholder.rgnList);
        }
        else if (255 == upperGray)
        {
            BrightThreshold pred(lowerGray, upperGray);
            GeneralThresholdTBB<BrightThreshold> thresholder(grayImage, pred);

            tbb::parallel_reduce(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
            rgnList.swap(thresholder.rgnList);
        }
        else
        {
            MundaneThreshold pred(lowerGray, upperGray);
            GeneralThresholdTBB<MundaneThreshold> thresholder(grayImage, pred);

            tbb::parallel_reduce(tbb::blocked_range<int>(0, grayImage.rows), thresholder);
            rgnList.swap(thresholder.rgnList);
        }

        std::sort(rgnList.begin(), rgnList.end(), [](const UPSpamRunListTBB &l, const UPSpamRunListTBB &r) { return l->front().l < r->front().l; });

        SpamRunListTBB::size_type numRuns = 0;
        for (const auto &rgnItem :rgnList)
        {
            numRuns += rgnItem->size();
        }

        runList.resize(numRuns);
        auto pDest = runList.data();
        for (const auto &rgnItem : rgnList)
        {
            std::memcpy(pDest, rgnItem->data(), rgnItem->size() * sizeof(rgnItem->front()));
            pDest += rgnItem->size();
        }
    }

    return rgn;
}