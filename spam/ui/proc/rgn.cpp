#include "rgn.h"
#include <limits>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#include <stack>

namespace
{
    constexpr int g_numRgnColors = 12;
    static const uint32_t g_rgnColors[g_numRgnColors] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
        0xFF800000, 0xFFFFFF00, 0xFF808000, 0xFF008000, 0xFF00FFFF, 0xFF008080, 0xFF000080, 0xFFFF00FF, 0xFF800080 };
};

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
                                *pPixel = color_;
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

SPSpamRgnVector SpamRgn::Connect() const
{
    AdjacencyList al = GetAdjacencyList();
    std::vector<uint8_t> met(al.size());

    std::vector<std::vector<int>> rgnIdxs;
    const int numRuns = static_cast<int>(al.size());
    for (int n=0; n<numRuns; ++n)
    {
        if (!met[n])
        {
            rgnIdxs.emplace_back();
            std::vector<int> runStack(1, n);
            while (!runStack.empty())
            {
                const int seedRun = runStack.back();
                runStack.pop_back();
                if (!met[seedRun])
                {
                    met[seedRun] = 1;
                    rgnIdxs.back().push_back(seedRun);
                    for (const int a : al[seedRun])
                    {
                        runStack.push_back(a);
                    }
                }
            }

            std::sort(rgnIdxs.back().begin(), rgnIdxs.back().end());
        }
    }

    SPSpamRgnVector rgs = std::make_shared<SpamRgnVector>();
    rgs->resize(rgnIdxs.size());
    const int numRgns = static_cast<int>(rgnIdxs.size());
    for (int r = 0; r < numRgns; ++r)
    {
        SpamRgn &rgn = (*rgs)[r];
        const std::vector<int> &rgnIdx = rgnIdxs[r];
        const int numRgnRuns = static_cast<int>(rgnIdx.size());
        rgn.data_.resize(numRgnRuns);
        for (int rr = 0; rr < numRgnRuns; ++rr)
        {
            rgn.data_[rr] = data_[rgnIdx[rr]];
        }

        rgn.color_ = g_rgnColors[r%g_numRgnColors];
    }

    return rgs;
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

AdjacencyList SpamRgn::GetAdjacencyList() const
{
    AdjacencyList al(data_.size());
    if (!data_.empty() && data_.back().l > 0)
    {
        const int numRuns = static_cast<int>(data_.size());
        std::vector<std::pair<int, int>> rowIndexRanges(data_.back().l+1);

        int prevRun = 0;
        rowIndexRanges[data_.front().l].first = 0;
        for (int run = 0; run<numRuns; ++run)
        {
            if (data_[run].l != data_[prevRun].l)
            {
                rowIndexRanges[data_[prevRun].l].second = run;
                rowIndexRanges[data_[run].l].first = run;
            }

            prevRun = run;
        }
        rowIndexRanges[data_[prevRun].l].second = numRuns;

        const int row0StartIndex    = rowIndexRanges[0].first;
        const int row0EndIndex      = rowIndexRanges[0].second;
        const int rowMidStartIndex  = rowIndexRanges[0].second;
        const int rowMidEndIndex    = rowIndexRanges[data_.back().l].first;
        const int rowLastStartIndex = rowIndexRanges[data_.back().l].first;
        const int rowLastEndIndex   = rowIndexRanges[data_.back().l].second;

        for (int i=row0StartIndex; i<row0EndIndex; ++i)
        {
            const int nextRow = data_[i].l + 1;
            for (int j = rowIndexRanges[nextRow].first; j < rowIndexRanges[nextRow].second; ++j)
            {
                if (IsRunColumnIntersection(data_[i], data_[j]))
                {
                    al[i].push_back(j);
                }
            }
        }

        for (int i = rowMidStartIndex; i<rowMidEndIndex; ++i)
        {
            const int prevRow = data_[i].l - 1;
            const int nextRow = data_[i].l + 1;

            for (int j = rowIndexRanges[prevRow].first; j < rowIndexRanges[prevRow].second; ++j)
            {
                if (IsRunColumnIntersection(data_[i], data_[j]))
                {
                    al[i].push_back(j);
                }
            }
            
            for (int j = rowIndexRanges[nextRow].first; j < rowIndexRanges[nextRow].second; ++j)
            {
                if (IsRunColumnIntersection(data_[i], data_[j]))
                {
                    al[i].push_back(j);
                }
            }
        }

        for (int i = rowLastStartIndex; i<rowLastEndIndex; ++i)
        {
            const int prevRow = data_[i].l - 1;
            for (int j = rowIndexRanges[prevRow].first; j < rowIndexRanges[prevRow].second; ++j)
            {
                if (IsRunColumnIntersection(data_[i], data_[j]))
                {
                    al[i].push_back(j);
                }
            }
        }
    }

    return al;
}