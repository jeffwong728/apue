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
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef WINDING
#undef WINDING
#endif
#include <cairomm/cairomm.h>
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
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::A8, sz.width);
    std::unique_ptr<uchar[]> imgData{ new uchar[sz.height*step]() };
    cv::Mat mask(sz.height, sz.width, CV_8UC1, imgData.get(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Surface::Format::A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
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
    int step = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::A8, sz.width);
    buf.resize(0);
    buf.resize(sz.height*step);

    cv::Mat mask(sz.height, sz.width, CV_8UC1, buf.data(), step);
    auto imgSurf = Cairo::ImageSurface::create(mask.data, Cairo::Surface::Format::A8, mask.cols, mask.rows, static_cast<int>(mask.step1()));
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