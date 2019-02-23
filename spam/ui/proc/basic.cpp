#include "basic.h"
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

struct DarkThreshold
{
    DarkThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) { return grayVal < upper; }
    const uchar lower;
    const uchar upper;
};

struct BrightThreshold
{
    BrightThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) { return grayVal > lower; }
    const uchar lower;
    const uchar upper;
};

struct MundaneThreshold
{
    MundaneThreshold(const uchar lowerGray, const uchar upperGray) : lower(lowerGray), upper(upperGray) {}
    bool operator()(const uchar grayVal) { return grayVal > lower && grayVal < upper; }
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
                        rgn->AddRun(r, cb, c );
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                rgn->AddRun(r, cb, right);
            }
        }
    }

    return rgn;
}

SPSpamRgn BasicImgProc::Threshold(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    if (0 == lowerGray)
    {
        return GeneralThreshold<DarkThreshold>(grayImage, lowerGray, upperGray);
    }
    else if (255 == upperGray)
    {
        return GeneralThreshold<BrightThreshold>(grayImage, lowerGray, upperGray);
    }
    else
    {
        return GeneralThreshold<MundaneThreshold>(grayImage, lowerGray, upperGray);
    }
}