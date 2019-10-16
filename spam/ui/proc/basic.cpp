#include "basic.h"
#include <asmlib.h>
#include <memory>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 )
#include <2geom/2geom.h>
#include <2geom/path-intersection.h>
#include <2geom/cairo-path-sink.h>
#pragma warning( pop )
#include <cairomm/cairomm.h>
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

struct CVPointLesser
{
    bool operator()(const cv::Point &a, const cv::Point &b) const
    {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    }
};

template <typename Pred>
class GeneralThresholdPI
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const Pred &pred, const int16_t b, const int16_t e)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e) {  }

public:
    void operator()() const
    {
        constexpr int16_t left = 0;
        const int16_t right = static_cast<int16_t>(grayImage_.cols);
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
                        rgn_.emplace_back(l, cb, c); cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {

                rgn_.emplace_back(l, cb, right);
            }
        }
    }

public:
    mutable SpamRunListTBB rgn_;
    const cv::Mat &grayImage_;
    const Pred &pred_;
    const int16_t beg_;
    const int16_t end_;
};

template<>
struct GeneralThresholdPI<SIMDThreshold>
{
public:
    GeneralThresholdPI(const cv::Mat &grayImage, const SIMDThreshold &pred, const int16_t b, const int16_t e)
        : grayImage_(grayImage), pred_(pred), beg_(b), end_(e) {  }

public:
    void operator()() const
    {
        const int16_t width = static_cast<int16_t>(grayImage_.cols);
        constexpr int16_t simdSize = 32;

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
                            rgn_.emplace_back(l, cb, c+i); cb = -1;
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
                        rgn_.emplace_back(l, cb, c); cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {
                rgn_.emplace_back(l, cb, width);
            }
        }
    }

public:
    mutable SpamRunListTBB rgn_;
    const cv::Mat &grayImage_;
    const SIMDThreshold &pred_;
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

struct RunLengthLabelEncoder
{
public:
    RunLengthLabelEncoder(const cv::Mat &labelImage, const int16_t b, const int16_t e)
        : labelImage_(labelImage), beg_(b), end_(e) {  }

public:
    void operator()() const
    {
        const int16_t width = static_cast<int16_t>(labelImage_.cols);
        constexpr int16_t simdSize = 8;
        constexpr uint32_t wSimdSize = simdSize;

        bool sOk[simdSize] = { false };
        const auto stride = labelImage_.step1();
        const int32_t* pRow = reinterpret_cast<const int32_t *>(labelImage_.data) + beg_ * stride;
        const int16_t regularWidth = width & (-simdSize);

        vcl::Vec8i blockPixels;
        for (auto l = beg_; l != end_; ++l)
        {
            uint16_t label = 0;
            int16_t c, cb = -1;
            const int32_t* pPixel = pRow;
            for (c = 0; c < regularWidth; c += simdSize, pPixel += simdSize)
            {
                blockPixels.load(pPixel);
                vcl::Vec8ib r = blockPixels > 0;
                auto numBright = vcl::horizontal_count(r);

                if (cb < 0 && !numBright)
                {
                    continue;
                }

                if (cb > 0 && wSimdSize == numBright)
                {
                    continue;
                }

                for (int16_t i = 0; i < simdSize; ++i)
                {
                    if (pPixel[i]) {
                        if (cb < 0) {
                            cb = c + i; label = pPixel[i];
                        }
                    }
                    else {
                        if (cb > -1) {
                            rgn_.emplace_back(l, cb, c + i, label); cb = -1;
                        }
                    }
                }
            }

            for (; c < width; ++c)
            {
                if (*pPixel) {
                    if (cb < 0) { cb = c; label = *pPixel; }
                }
                else {
                    if (cb > -1) {
                        rgn_.emplace_back(l, cb, c, label); cb = -1;
                    }
                }
                pPixel += 1;
            }

            pRow += stride;

            if (cb > -1) {
                rgn_.emplace_back(l, cb, width, label);
            }
        }
    }

public:
    mutable SpamRunListTBB rgn_;
    const cv::Mat &labelImage_;
    const int16_t beg_;
    const int16_t end_;
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
            fObjs.emplace_back(grayImage, pred, rowStep*t, rowStep*(t + 1));
        }
        fObjs.emplace_back(grayImage, pred, rowStep*(numTasks - 1), grayImage.rows);

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
            numRuns += fObj.rgn_.size();
        }

        runList.resize(numRuns);
        auto pDest = runList.data();
        for (const auto &fObj : fObjs)
        {
            ::A_memcpy(pDest, fObj.rgn_.data(), fObj.rgn_.size() * sizeof(fObj.rgn_.front()));
            pDest += fObj.rgn_.size();
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
}

void BasicImgProc::ReturnRegion(SpamRunList &&rgn)
{
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

cv::Mat BasicImgProc::Binarize(const cv::Mat &grayImage, const uchar lowerGray, const uchar upperGray)
{
    cv::Mat binImg;
    if (0 == lowerGray)
    {
        cv::threshold(grayImage, binImg, upperGray, 255, cv::THRESH_BINARY_INV);
    }
    else if (255 == upperGray)
    {
        cv::threshold(grayImage, binImg, lowerGray, 255, cv::THRESH_BINARY);
    }
    else
    {
        cv::inRange(grayImage, lowerGray, upperGray, binImg);
    }

    return binImg;
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

    int dph = labels.depth();
    int cnl = labels.channels();

    if (CV_32S == dph && 1 == cnl)
    {
        const int numTasks = std::min(10, static_cast<int>(BasicImgProc::s_runList_pools_.size()));
        const int rowStep = labels.rows / numTasks;
        boost::container::static_vector<RunLengthLabelEncoder, 10> fObjs;
        for (int t = 0; t < numTasks - 1; ++t)
        {
            fObjs.emplace_back(labels, rowStep*t, rowStep*(t + 1));
        }
        fObjs.emplace_back(labels, rowStep*(numTasks - 1), labels.rows);

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

        std::vector<int> numRunsOfRgn(numLabels+1);
        SpamRunListTBB::size_type numRuns = 0;
        for (const auto &fObj : fObjs)
        {
            for (const auto &run : fObj.rgn_)
            {
                numRunsOfRgn[run.label] += 1;
            }
        }

        int rgnIdx = 1;
        for (SpamRgn &rgn : rgns)
        {
            rgn.GetData().reserve(numRunsOfRgn[rgnIdx++]+2);
        }

        for (const auto &fObj : fObjs)
        {
            for (const auto &run : fObj.rgn_)
            {
                rgns[run.label-1].GetData().emplace_back(run);
            }
        }
    }

    return rgs;
}

cv::Mat BasicImgProc::PathToMask(const Geom::PathVector &pv, const cv::Size &sz)
{
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Format::FORMAT_A8, sz.width);
    std::unique_ptr<uchar[]> imgData{ new uchar[sz.height*step]() };
    cv::Mat mask(sz.height, sz.width, CV_8UC1, imgData.get(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Format::FORMAT_A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
    auto cr = Cairo::Context::create(imgSurf);
    cr->translate(0.5, 0.5);

    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->fill();

    mask = mask.clone();
    return mask;
}

cv::Mat BasicImgProc::PathToMask(const Geom::PathVector &pv, const cv::Size &sz, std::vector<uint8_t> &buf)
{
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Format::FORMAT_A8, sz.width);
    buf.resize(0);
    buf.resize(sz.height*step);

    cv::Mat mask(sz.height, sz.width, CV_8UC1, buf.data(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Format::FORMAT_A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
    auto cr = Cairo::Context::create(imgSurf);
    cr->translate(0.5, 0.5);

    Geom::CairoPathSink cairoPathSink(cr->cobj());
    cairoPathSink.feed(pv);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    cr->fill();

    return mask;
}

void BasicImgProc::Transform(const cv::Mat &grayImage, cv::Mat &dst, const cv::Mat &transMat, const cv::Rect &mask)
{
    dst.create(grayImage.rows, grayImage.cols, grayImage.type());
    dst = 0;

    const int rowBeg = std::max(0, mask.y);
    const int colBeg = std::max(0, mask.x);
    const int rowEnd = std::min(grayImage.rows, mask.y + mask.height);
    const int colEnd = std::min(grayImage.cols, mask.x + mask.width);

    std::vector<cv::Point2f> maskPoints;
    if (rowBeg < rowEnd && colBeg < colEnd)
    {
        maskPoints.reserve((rowEnd - rowBeg) * (colEnd - colBeg));
    }

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        for (int col = colBeg; col < colEnd; ++col)
        {
            maskPoints.emplace_back(static_cast<float>(col), static_cast<float>(row));
        }
    }

    std::vector<cv::Point2f> maskSrcPts;
    cv::transform(maskPoints, maskSrcPts, transMat);

    const cv::Point2f *pMaskSrcPt = maskSrcPts.data();
    for (int row = rowBeg; row < rowEnd; ++row)
    {
        for (int col = colBeg; col < colEnd; ++col)
        {
            const int16_t grayVal = getGrayScaleSubpix(grayImage, *pMaskSrcPt);
            dst.at<uint8_t>(row, col) = cv::saturate_cast<uint8_t>(grayVal);
            pMaskSrcPt += 1;
        }
    }
}

void BasicImgProc::TrackCurves(const std::vector<cv::Point> &points,
    const cv::Point &minPoint,
    const cv::Point &maxPoint,
    std::vector<std::vector<int>> &curves,
    std::vector<bool> &closed)
{
    curves.clear();
    const int width = maxPoint.x - minPoint.x + 1;
    const int height = maxPoint.y - minPoint.y + 1;
    cv::Mat tmplMat = cv::Mat::zeros(height, width, CV_8UC1);

    int i = 0;
    std::map<cv::Point, int, CVPointLesser> pointIndicesDict;
    for (const cv::Point &pt : points)
    {
        const cv::Point tPt = pt - minPoint;
        tmplMat.at<uint8_t>(tPt) = 0xFF;
        pointIndicesDict[tPt] = i++ ;
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(tmplMat, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

    tmplMat = cv::Scalar();
    for (const std::vector<cv::Point> &contour : contours)
    {
        std::vector<int> curve;
        for (const cv::Point &pt : contour)
        {
            uint8_t &visted = tmplMat.at<uint8_t>(pt);
            if (visted)
            {
                if (curve.size() > 3)
                {
                    curves.push_back(std::move(curve));
                }
                else
                {
                    curve.resize(0);
                }
            }
            else
            {
                visted = 0xFF;
                curve.push_back(pointIndicesDict[pt]);
            }
        }

        if (!curve.empty())
        {
            curves.push_back(std::move(curve));
        }
    }

    const auto &isEndNear = [](const cv::Point &pt1, const cv::Point &pt2) { return std::abs(pt1.x-pt2.x) < 3 && std::abs(pt1.y - pt2.y) < 3; };
    for (;;)
    {
        const int numCurves = static_cast<int>(curves.size());
        int i = 0, j = 0, k=-1;
        for (; i < numCurves-1; ++i)
        {
            for (j = i+1; j < numCurves; ++j)
            {
                if (isEndNear(points[curves[i].back()], points[curves[j].front()]))
                {
                    k = 0; break;
                }
                else if (isEndNear(points[curves[i].back()], points[curves[j].back()]))
                {
                    k = 1; break;
                }
                else if (isEndNear(points[curves[i].front()], points[curves[j].front()]))
                {
                    k = 2; break;
                }
                else if (isEndNear(points[curves[i].front()], points[curves[j].back()]))
                {
                    k = 3; break;
                }
                else
                {
                    k = -1;
                }
            }

            if (j < numCurves)
            {
                break;
            }
        }

        if (i < numCurves-1)
        {
            if (k >= 0)
            {
                curves[j].swap(curves[curves.size() - 1]);
                curves[i].swap(curves[curves.size() - 2]);
            }

            switch (k)
            {
            case 1:
                std::reverse(curves.back().begin(), curves.back().end());
                break;

            case 2:
                std::reverse(curves[curves.size() - 2].begin(), curves[curves.size() - 2].end());
                break;

            case 3:
                std::reverse(curves[curves.size() - 2].begin(), curves[curves.size() - 2].end());
                std::reverse(curves.back().begin(), curves.back().end());
                break;

            default: break;
            }

            if (k >= 0)
            {
                curves[curves.size() - 2].insert(curves[curves.size() - 2].end(), curves.back().cbegin(), curves.back().cend());
                curves.pop_back();
            }
        }
        else
        {
            break;
        }
    }

    closed.resize(0);
    for (const std::vector<int> &curve : curves)
    {
        if (curve.size()>1)
        {
            if (isEndNear(points[curve.front()], points[curve.back()]))
            {
                closed.push_back(true);
            }
            else
            {
                closed.push_back(false);
            }
        }
        else
        {
            closed.push_back(false);
        }
    }
}

void BasicImgProc::SplitCurvesToSegments(const std::vector<bool> &closed, std::vector<std::vector<int>> &curves)
{
    std::vector<std::vector<int>> segments;
    constexpr int simdSize = 8;
    const int numCurves = static_cast<int>(curves.size());
    for (int i=0; i<numCurves; ++i)
    {
        const std::vector<int> &curve = curves[i];
        const int numEdges = static_cast<int>(curve.size());
        const int regularNumEdges = numEdges & (-simdSize);
        for (int e = 0; e < regularNumEdges; e += simdSize)
        {
            segments.emplace_back();
            segments.back().reserve(simdSize);
            for (int i = 0; i < simdSize; ++i)
            {
                segments.back().push_back(curve[e+i]);
            }
        }

        if (closed[i])
        {
            segments.emplace_back();
            segments.back().reserve(simdSize);
            for (int e = regularNumEdges; e < regularNumEdges + simdSize; ++e)
            {
                segments.back().push_back(curve[e%numEdges]);
            }
        }
        else
        {
            const int iregularNumEdges = numEdges - regularNumEdges;
            if (iregularNumEdges > 3)
            {
                segments.emplace_back();
                segments.back().reserve(simdSize);
                for (int e = regularNumEdges; e < numEdges; ++e)
                {
                    segments.back().push_back(curve[e]);
                }

                for (int e = regularNumEdges; e < regularNumEdges + simdSize - iregularNumEdges; ++e)
                {
                    segments.back().push_back(curve[e]);
                }
            }
        }
    }

    curves.swap(segments);
}

void filter2D_Conv(cv::InputArray src, cv::OutputArray dst, int ddepth,
    cv::InputArray kernel, cv::Point anchor = cv::Point(-1, -1),
    double delta = 0, int borderType = cv::BORDER_DEFAULT)
{
    cv::Mat newKernel;
    const int FLIP_H_Z = -1;
    cv::flip(kernel, newKernel, FLIP_H_Z);
    cv::Point newAnchor = anchor;
    if (anchor.x > 0 && anchor.y >= 0)
        newAnchor = cv::Point(newKernel.cols - anchor.x - 1, newKernel.rows - anchor.y - 1);
    cv::filter2D(src, dst, ddepth, newKernel, newAnchor, delta, borderType);
}

float GuassianValue2D(float ssq, float x, float y)
{
    return std::exp(-(x*x + y * y) / (2.0f *ssq)) / (2.0f * static_cast<float>(CV_PI) * ssq);
}

template<typename _tp>
void meshgrid(float xStart, float xInterval, float xEnd, float yStart, float yInterval, float yEnd, cv::Mat &matX, cv::Mat &matY)
{
    std::vector<_tp> vectorX, vectorY;
    _tp xValue = xStart;
    while (xValue <= xEnd) {
        vectorX.push_back(xValue);
        xValue += xInterval;
    }

    _tp yValue = yStart;
    while (yValue <= yEnd) {
        vectorY.push_back(yValue);
        yValue += yInterval;
    }
    cv::Mat matCol(vectorX);
    matCol = matCol.reshape(1, 1);

    cv::Mat matRow(vectorY);
    matRow = matRow.reshape(1, static_cast<int>(vectorY.size()));
    matX = cv::repeat(matCol, static_cast<int>(vectorY.size()), 1);
    matY = cv::repeat(matRow, 1, static_cast<int>(vectorX.size()));
}

int _refineWithLMIteration(const cv::Mat &mat, cv::Mat &matTmpl, cv::Point2f &ptResult)
{
    cv::Mat matGuassian;
    int width = 2;
    float ssq = 1.;
    matGuassian.create(width * 2 + 1, width * 2 + 1, CV_32FC1);
    cv::Mat matI, matT;
    mat.convertTo(matI, CV_32FC1);
    matTmpl.convertTo(matT, CV_32FC1);

    cv::Mat matX, matY;
    const float fWidth = static_cast<float>(width);
    meshgrid<float>(-fWidth, 1.f, fWidth, -fWidth, 1.f, fWidth, matX, matY);
    for (int row = 0; row < matX.rows; ++row)
        for (int col = 0; col < matX.cols; ++col)
        {
            matGuassian.at<float>(row, col) = GuassianValue2D(ssq, matX.at<float>(row, col), matY.at<float>(row, col));
        }
    matGuassian = matGuassian.mul(-matX);
    cv::Mat matTmp(matGuassian, cv::Range::all(), cv::Range(0, 2));
    double fSum = cv::sum(matTmp)[0];
    cv::Mat matGuassianKernalX, matGuassianKernalY;
    matGuassianKernalX = matGuassian / fSum;        //XSG question, the kernel is reversed?
    cv::transpose(matGuassianKernalX, matGuassianKernalY);

    /**************** Using LM Iteration ****************/
    int N = 0, v = 2;
    cv::Mat matD;
    matD.create(2, 1, CV_32FC1);
    matD.at<float>(0, 0) = ptResult.x;
    matD.at<float>(1, 0) = ptResult.y;

    cv::Mat matDr = matD.clone();

    cv::Mat matInputNew;

    auto interp2 = [matI, matT](cv::Mat &matOutput, const cv::Mat &matD) {
        cv::Mat map_x, map_y;
        map_x.create(matT.size(), CV_32FC1);
        map_y.create(matT.size(), CV_32FC1);
        cv::Point2f ptStart(matD.at<float>(0, 0), matD.at<float>(1, 0));
        for (int row = 0; row < matT.rows; ++row)
            for (int col = 0; col < matT.cols; ++col)
            {
                map_x.at<float>(row, col) = ptStart.x + col;
                map_y.at<float>(row, col) = ptStart.y + row;
            }
        cv::remap(matI, matOutput, map_x, map_y, cv::INTER_LINEAR);
    };

    interp2(matInputNew, matD);

    cv::Mat matR = matT - matInputNew;
    cv::Mat matRn = matR.clone();
    double fRSum = cv::sum(matR.mul(matR))[0];
    double fRSumN = fRSum;

    cv::Mat matDerivativeX, matDerivativeY;
    filter2D_Conv(matInputNew, matDerivativeX, CV_32F, matGuassianKernalX, cv::Point(-1, -1), 0.0, cv::BORDER_REPLICATE);
    filter2D_Conv(matInputNew, matDerivativeY, CV_32F, matGuassianKernalY, cv::Point(-1, -1), 0.0, cv::BORDER_REPLICATE);

    cv::Mat matRt = matR.reshape(1, 1);
    cv::Mat matRtTranspose;
    cv::transpose(matRt, matRtTranspose);
    matDerivativeX = matDerivativeX.reshape(1, 1);
    matDerivativeY = matDerivativeY.reshape(1, 1);

    const float* p = matDerivativeX.ptr<float>(0);
    std::vector<float> vecDerivativeX(p, p + matDerivativeX.cols);

    cv::Mat matJacobianT, matJacobian;
    matJacobianT.push_back(matDerivativeX);
    matJacobianT.push_back(matDerivativeY);
    cv::transpose(matJacobianT, matJacobian);

    cv::Mat matE = cv::Mat::eye(2, 2, CV_32FC1);

    cv::Mat A = matJacobianT * matJacobian;
    cv::Mat g = -matJacobianT * matRtTranspose;

    double min, max;
    cv::minMaxLoc(A, &min, &max);
    double mu = 1. * max;
    double err1 = 1e-4, err2 = 1e-4;
    auto Nmax = 100;
    while (cv::norm(matDr) > err2 && N < Nmax) {
        ++N;
        cv::solve(A + mu * matE, -g, matDr);     // equal to matlab matDr = (A+mu*E)\(-g);

        cv::Mat matDn = matD + matDr;
        if (cv::norm(matDr) < err2) {
            interp2(matInputNew, matDn);
            matRn = matT - matInputNew;
            fRSumN = cv::sum(matR.mul(matR))[0];
            matD = matDn;
            break;
        }
        else {
            if (matDn.at<float>(0, 0) > matI.cols - matT.cols ||
                matDn.at<float>(0, 0) < 0 ||
                matDn.at<float>(1, 0) > matI.rows - matT.rows ||
                matDn.at<float>(1, 0) < 0) {
                mu *= v;
                v *= 2;
            }
            else {
                interp2(matInputNew, matDn);
                matRn = matT - matInputNew;
                fRSumN = cv::sum(matRn.mul(matRn))[0];

                cv::Mat matDrTranspose;
                cv::transpose(matDr, matDrTranspose);
                cv::Mat matL = (matDrTranspose * (mu * matDr - g));   // L(0) - L(hlm) = 0.5 * h' ( uh - g)
                double L = matL.at<float>(0, 0);
                double F = fRSum - fRSumN;
                double rho = F / L;

                if (rho > 0) {
                    matD = matDn.clone();
                    matR = matRn.clone();
                    fRSum = fRSumN;

                    filter2D_Conv(matInputNew, matDerivativeX, CV_32F, matGuassianKernalX, cv::Point(-1, -1), 0.0, cv::BORDER_REPLICATE);
                    filter2D_Conv(matInputNew, matDerivativeY, CV_32F, matGuassianKernalY, cv::Point(-1, -1), 0.0, cv::BORDER_REPLICATE);
                    matRt = matR.reshape(1, 1);
                    cv::transpose(matRt, matRtTranspose);

                    matDerivativeX = matDerivativeX.reshape(1, 1);
                    matDerivativeY = matDerivativeY.reshape(1, 1);

                    matJacobianT.release();
                    matJacobianT.push_back(matDerivativeX);
                    matJacobianT.push_back(matDerivativeY);
                    cv::transpose(matJacobianT, matJacobian);

                    A = matJacobianT * matJacobian;
                    g = -matJacobianT * matRtTranspose;

                    mu *= std::max(1. / 3., 1 - pow(2 * rho - 1, 3));
                }
                else {
                    mu *= v; v *= 2;
                }
            }
        }
    }

    ptResult.x = matD.at<float>(0, 0);
    ptResult.y = matD.at<float>(1, 0);
    return 0;
}

int matchTemplate(const cv::Mat &mat, cv::Mat &matTmpl, cv::Point2f &ptResult)
{
    cv::Mat img_display, matResult;
    const int match_method = cv::TM_SQDIFF;

    mat.copyTo(img_display);

    /// Create the result matrix
    int result_cols = mat.cols - matTmpl.cols + 1;
    int result_rows = mat.rows - matTmpl.rows + 1;

    matResult.create(result_rows, result_cols, CV_32FC1);

    /// Do the Matching and Normalize
    cv::matchTemplate(mat, matTmpl, matResult, match_method);
    cv::normalize(matResult, matResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal;
    cv::Point minLoc, maxLoc, matchLoc;

    cv::minMaxLoc(matResult, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if (match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
        matchLoc = minLoc;
    else
        matchLoc = maxLoc;

    ptResult.x = (float)matchLoc.x;
    ptResult.y = (float)matchLoc.y;
    _refineWithLMIteration(mat, matTmpl, ptResult);

    ptResult.x += (float)(matTmpl.cols / 2 + 0.5);
    ptResult.y += (float)(matTmpl.rows / 2 + 0.5);

    return 0;
}