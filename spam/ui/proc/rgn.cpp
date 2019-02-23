#include "rgn.h"
#include <limits>
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

void SpamRgn::Draw(const cv::Mat &dstImage, const double sx, const double sy) const
{
    int dph = dstImage.depth();
    int cnl = dstImage.channels();
    if (CV_8U == dph && 4 == cnl)
    {
        cv::Rect bbox = BoundingBox();
        double minx = sx * bbox.x;
        double miny = sy * bbox.y;
        double maxx = sx * (bbox.x + bbox.width + 1);
        double maxy = sy * (bbox.y + bbox.height + 1);
        for (double y = miny; y < maxy; ++y)
        {
            int or = cv::saturate_cast<int>(y / sy - 0.5);
            auto lb = std::lower_bound(data_.cbegin(), data_.cend(), or, [](const SpamRun &run, const int val) { return  run.l < val; });
            if (lb != data_.cend() && or == lb->l)
            {
                int r = cv::saturate_cast<int>(y);
                if (r >= 0 && r<dstImage.rows)
                {
                    auto pRow = dstImage.data + r * dstImage.step1();
                    for (double x = minx; x<maxx; ++x)
                    {
                        int oc = cv::saturate_cast<int>(x / sx - 0.5);
                        auto itRun = lb;
                        bool cInside = false;
                        while (itRun != data_.cend() && or == itRun->l)
                        {
                            if (oc < itRun->ce && oc >= itRun->cb)
                            {
                                cInside = true;
                                break;
                            }

                            itRun += 1;
                        }

                        if (cInside)
                        {
                            int c = cv::saturate_cast<int>(x);
                            if (c >= 0 && c<dstImage.cols)
                            {
                                auto pPixel = reinterpret_cast<uint32_t *>(pRow + c * 4);
                                *pPixel = 0xFFFF0000;
                            }
                        }
                    }
                }
            }
        }
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

cv::Rect SpamRgn::BoundingBox() const
{
    cv::Point minPoint{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    cv::Point maxPoint{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

    for (const SpamRun &r : data_)
    {
        if (r.l < minPoint.y)
        {
            minPoint.y = r.l;
        }

        if (r.l > maxPoint.y)
        {
            maxPoint.y = r.l;
        }

        if (r.cb < minPoint.x)
        {
            minPoint.x = r.cb;
        }

        if (r.ce > maxPoint.x)
        {
            maxPoint.x = r.ce;
        }
    }

    if (data_.empty())
    {
        return cv::Rect();
    }
    else
    {
        return cv::Rect(minPoint, maxPoint);
    }
}

bool SpamRgn::Contain(const int r, const int c) const
{
    auto lb = std::lower_bound(data_.cbegin(), data_.cend(), r, [](const SpamRun &run, const int val) { return val < run.l; });
    while (lb != data_.cend() && r == lb->l)
    {
        if (c<lb->ce && c>=lb->cb)
        {
            return true;
        }

        lb += 1;
    }

    return false;
}