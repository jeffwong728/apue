#include "rgn.h"
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>

class RunLengthEncoder
{
public:
    RunLengthEncoder(const cv::Mat &binaryImage)
        : firstPixel_(binaryImage.data)
        , stride_(binaryImage.step1())
        , width_(binaryImage.cols)
    {}

    RunLengthEncoder(RunLengthEncoder& x, tbb::split)
        : firstPixel_(x.firstPixel_)
        , stride_(x.stride_)
        , width_(x.width_)
    {}

    void operator()(const tbb::blocked_range<int>& br)
    {
        int left = 0;
        int right = width_;
        int end = br.end();
        for (int r = br.begin(); r != end; ++r)
        {
            int cb = -1;
            const uchar* pRow = firstPixel_ + r * stride_;
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
                        runs_.push_back({ r, cb, c });
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                runs_.push_back({ r, cb, right });
            }
        }
    }

    void join(const RunLengthEncoder& y) { runs_.insert(runs_.begin(), y.runs_.cbegin(), y.runs_.cend()); }
    std::vector<SpamRun> &Runs() { return runs_; }

private:
    const uchar* const firstPixel_;
    const size_t stride_;
    const int width_;
    std::vector<SpamRun> runs_;
};

void SpamRgn::AddRun(const cv::Mat &binaryImage)
{
    int dph = binaryImage.depth();
    int cnl = binaryImage.channels();
    if (CV_8U == dph && 1 == cnl)
    {
        int top = 0;
        int bot = binaryImage.rows;
        int left = 0;
        int right = binaryImage.cols;
        for (int r = top; r<bot; ++r)
        {
            int cb = -1;
            const uchar* pRow = binaryImage.data + r * binaryImage.step1();
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
                        data_.push_back({ r, cb, c });
                        cb = -1;
                    }
                }
            }

            if (cb > -1)
            {
                data_.push_back({ r, cb, right });
            }
        }
    }
}

void SpamRgn::AddRunParallel(const cv::Mat &binaryImage)
{
    int dph = binaryImage.depth();
    int cnl = binaryImage.channels();
    if (CV_8U == dph && 1 == cnl)
    {
        RunLengthEncoder enc(binaryImage);
        tbb::parallel_reduce(tbb::blocked_range<int>(0, binaryImage.rows), enc);
        data_.swap(enc.Runs());
    }
}

double SpamRgn::Area() const
{
    double a = 0;
    for (const SpamRun &r : data_)
    {
        a += (r.ce - r.cb);
    }

    return a;
}

void SpamRgn::Connect() const
{

}