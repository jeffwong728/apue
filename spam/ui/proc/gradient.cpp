#include "gradient.h"
#include <memory>
#include <vectorclass/vectorclass.h>
#include <boost/container/static_vector.hpp>
#pragma warning( push )
#pragma warning( disable : 4819 4003 4267 4244 )
#include <2geom/2geom.h>
#include <2geom/path-intersection.h>
#include <2geom/cairo-path-sink.h>
#pragma warning( pop )
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>

struct SimpleGradient
{
    SimpleGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd)
    : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd) {}

    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

struct SimpleNormGradient
{
    SimpleNormGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd)
        : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd) {}

    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

struct SobelGradient
{
    SobelGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd)
        : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd) {}

    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

void SimpleGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    const int width = colEnd - colBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularEnd   = colBeg + regularWidth;

    const auto imgStep = img->step1();
    const auto dxStep  = dx->step1();
    const auto dyStep  = dy->step1();
    const uint8_t *pImgRow = img->ptr<uint8_t>(rowBeg);
    int16_t *pDXRow      = dx->ptr<int16_t>(rowBeg);
    int16_t *pDYRow      = dy->ptr<int16_t>(rowBeg);
    vcl::Vec16uc prevVals, nextVals, aboveVals, belowVals;

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        int16_t *pDXCol = pDXRow;
        int16_t *pDYCol = pDYRow;
        const uint8_t *pImgCol = pImgRow;

        for (int col = colBeg; col < regularEnd; col += vectorSize)
        {
            prevVals.load(pImgCol - 1);
            nextVals.load(pImgCol + 1);

            vcl::Vec16s prevX = vcl::extend(prevVals);
            vcl::Vec16s nextX = vcl::extend(nextVals);
            vcl::Vec16s X = nextX - prevX;
            X.store(pDXCol);

            aboveVals.load(pImgCol - imgStep);
            belowVals.load(pImgCol + imgStep);

            vcl::Vec16s aboveY = vcl::extend(aboveVals);
            vcl::Vec16s belowY = vcl::extend(belowVals);
            vcl::Vec16s Y = belowY - aboveY;
            Y.store(pDYCol);

            pImgCol += vectorSize;
            pDXCol  += vectorSize;
            pDYCol  += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;
            prevVals.load_partial(lastCols, pImgCol - 1);
            nextVals.load_partial(lastCols, pImgCol + 1);

            vcl::Vec16s prevX = vcl::extend(prevVals);
            vcl::Vec16s nextX = vcl::extend(nextVals);
            vcl::Vec16s X = nextX - prevX;
            X.store_partial(lastCols, pDXCol);

            aboveVals.load_partial(lastCols, pImgCol - imgStep);
            belowVals.load_partial(lastCols, pImgCol + imgStep);

            vcl::Vec16s aboveY = vcl::extend(aboveVals);
            vcl::Vec16s belowY = vcl::extend(belowVals);
            vcl::Vec16s Y = belowY - aboveY;
            Y.store_partial(lastCols, pDYCol);
        }

        pImgRow += imgStep;
        pDXRow  += dxStep;
        pDYRow  += dyStep;
    }
}

void SimpleNormGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    constexpr int halfVectorSize = vectorSize/2;
    const int width = colEnd - colBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularEnd = colBeg + regularWidth;

    const auto imgStep = img->step1();
    const auto dxStep = dx->step1();
    const auto dyStep = dy->step1();
    const uint8_t *pImgRow = img->ptr<uint8_t>(rowBeg);
    float *pDXRow = dx->ptr<float>(rowBeg);
    float *pDYRow = dy->ptr<float>(rowBeg);

    vcl::Vec8i ones8i(1, 1, 1, 1, 1, 1, 1, 1);
    vcl::Vec16uc prevVals, nextVals, aboveVals, belowVals;

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        float *pDXCol = pDXRow;
        float *pDYCol = pDYRow;
        const uint8_t *pImgCol = pImgRow;

        for (int col = colBeg; col < regularEnd; col += vectorSize)
        {
            prevVals.load(pImgCol - 1);
            nextVals.load(pImgCol + 1);

            vcl::Vec16s prevX = vcl::extend(prevVals);
            vcl::Vec16s nextX = vcl::extend(nextVals);
            vcl::Vec16s X     = nextX - prevX;
            vcl::Vec8i  lowX  = vcl::extend(X.get_low());
            vcl::Vec8i  highX = vcl::extend(X.get_high());

            aboveVals.load(pImgCol - imgStep);
            belowVals.load(pImgCol + imgStep);

            vcl::Vec16s aboveY = vcl::extend(aboveVals);
            vcl::Vec16s belowY = vcl::extend(belowVals);
            vcl::Vec16s Y = belowY - aboveY;
            vcl::Vec8i  lowY = vcl::extend(Y.get_low());
            vcl::Vec8i  highY = vcl::extend(Y.get_high());

            vcl::Vec8i lowSqrSum = lowX * lowX + lowY * lowY;
            vcl::Vec8i lowNormSqrSum = vcl::select(lowSqrSum==0, ones8i, lowSqrSum);
            vcl::Vec8f lowRSqrt = vcl::approx_rsqrt(vcl::to_float(lowNormSqrSum));
            vcl::Vec8f lowNX = vcl::to_float(lowX) * lowRSqrt;
            vcl::Vec8f lowNY = vcl::to_float(lowY) * lowRSqrt;
            lowNX.store(pDXCol);
            lowNY.store(pDYCol);

            vcl::Vec8i highSqrSum = highX * highX + highY * highY;
            vcl::Vec8i highNormSqrSum = vcl::select(highSqrSum == 0, ones8i, highSqrSum);
            vcl::Vec8f highRSqrt = vcl::approx_rsqrt(vcl::to_float(highNormSqrSum));
            vcl::Vec8f highNX = vcl::to_float(highX) * highRSqrt;
            vcl::Vec8f highNY = vcl::to_float(highY) * highRSqrt;
            highNX.store(pDXCol + halfVectorSize);
            highNY.store(pDYCol + halfVectorSize);

            pImgCol += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;
            prevVals.load_partial(lastCols, pImgCol - 1);
            nextVals.load_partial(lastCols, pImgCol + 1);

            vcl::Vec16s prevX = vcl::extend(prevVals);
            vcl::Vec16s nextX = vcl::extend(nextVals);
            vcl::Vec16s X = nextX - prevX;
            vcl::Vec8i  lowX = vcl::extend(X.get_low());

            aboveVals.load_partial(lastCols, pImgCol - imgStep);
            belowVals.load_partial(lastCols, pImgCol + imgStep);

            vcl::Vec16s aboveY = vcl::extend(aboveVals);
            vcl::Vec16s belowY = vcl::extend(belowVals);
            vcl::Vec16s Y = belowY - aboveY;
            vcl::Vec8i  lowY = vcl::extend(Y.get_low());

            vcl::Vec8i lowSqrSum = lowX * lowX + lowY * lowY;
            vcl::Vec8i lowNormSqrSum = vcl::select(lowSqrSum == 0, ones8i, lowSqrSum);
            vcl::Vec8f lowRSqrt = vcl::approx_rsqrt(vcl::to_float(lowNormSqrSum));
            vcl::Vec8f lowNX = vcl::to_float(lowX) * lowRSqrt;
            vcl::Vec8f lowNY = vcl::to_float(lowY) * lowRSqrt;

            if (lastCols == halfVectorSize)
            {
                lowNX.store(pDXCol);
                lowNY.store(pDYCol);
            }
            else if (lastCols < halfVectorSize)
            {
                lowNX.store_partial(lastCols, pDXCol);
                lowNY.store_partial(lastCols, pDYCol);
            }
            else
            {
                lowNX.store(pDXCol);
                lowNY.store(pDYCol);
                vcl::Vec8i  highX = vcl::extend(X.get_high());
                vcl::Vec8i  highY = vcl::extend(Y.get_high());
                vcl::Vec8i  highSqrSum = highX * highX + highY * highY;
                vcl::Vec8i  highNormSqrSum = vcl::select(highSqrSum == 0, ones8i, highSqrSum);
                vcl::Vec8f  highRSqrt = vcl::approx_rsqrt(vcl::to_float(highNormSqrSum));
                vcl::Vec8f  highNX = vcl::to_float(highX) * highRSqrt;
                vcl::Vec8f  highNY = vcl::to_float(highY) * highRSqrt;
                highNX.store_partial(lastCols - halfVectorSize, pDXCol + halfVectorSize);
                highNY.store_partial(lastCols - halfVectorSize, pDYCol + halfVectorSize);
            }
        }

        pImgRow += imgStep;
        pDXRow += dxStep;
        pDYRow += dyStep;
    }
}

void SobelGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    const int width = colEnd - colBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularEnd = colBeg + regularWidth;

    const auto imgStep = img->step1();
    const auto dxStep = dx->step1();
    const auto dyStep = dy->step1();
    const uint8_t *pImgRow[3]{ img->ptr<uint8_t>(rowBeg - 1) + 1, img->ptr<uint8_t>(rowBeg) + 1, img->ptr<uint8_t>(rowBeg + 1) + 1 };
    int16_t *pDXRow = dx->ptr<int16_t>(rowBeg) + 1;
    int16_t *pDYRow = dy->ptr<int16_t>(rowBeg) + 1;
    vcl::Vec16uc prevVals[3], nextVals[3];

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        int16_t *pDXCol = pDXRow;
        int16_t *pDYCol = pDYRow;
        const uint8_t *pImgCol[3] = { pImgRow[0], pImgRow[1], pImgRow[2] };

        for (int col = colBeg; col < regularEnd; col += vectorSize)
        {
            prevVals[0].load(pImgCol[0] - 1); nextVals[0].load(pImgCol[0] + 1);
            prevVals[1].load(pImgCol[1] - 1); nextVals[1].load(pImgCol[1] + 1);
            prevVals[2].load(pImgCol[2] - 1); nextVals[2].load(pImgCol[2] + 1);

            vcl::Vec16s X = vcl::extend(nextVals[0]) - vcl::extend(prevVals[0]);
            X = X + 2 * (vcl::extend(nextVals[1]) - vcl::extend(prevVals[1]));
            X = X + vcl::extend(nextVals[2]) - vcl::extend(prevVals[2]);
            X.store(pDXCol);

            prevVals[0].load(pImgCol[0] - 1); nextVals[0].load(pImgCol[2] - 1);
            prevVals[1].load(pImgCol[0]); nextVals[1].load(pImgCol[2]);
            prevVals[2].load(pImgCol[0] + 1); nextVals[2].load(pImgCol[2] + 1);

            vcl::Vec16s Y = vcl::extend(nextVals[0]) - vcl::extend(prevVals[0]);
            Y = Y + 2 * (vcl::extend(nextVals[1]) - vcl::extend(prevVals[1]));
            Y = Y + vcl::extend(nextVals[2]) - vcl::extend(prevVals[2]);
            Y.store(pDYCol);

            pImgCol[0] += vectorSize;
            pImgCol[1] += vectorSize;
            pImgCol[2] += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;
            prevVals[0].load_partial(lastCols, pImgCol[0] - 1); nextVals[0].load_partial(lastCols, pImgCol[0] + 1);
            prevVals[1].load_partial(lastCols, pImgCol[1] - 1); nextVals[1].load_partial(lastCols, pImgCol[1] + 1);
            prevVals[2].load_partial(lastCols, pImgCol[2] - 1); nextVals[2].load_partial(lastCols, pImgCol[2] + 1);

            vcl::Vec16s X = vcl::extend(nextVals[0]) - vcl::extend(prevVals[0]);
            X = X + 2 * (vcl::extend(nextVals[1]) - vcl::extend(prevVals[1]));
            X = X + vcl::extend(nextVals[2]) - vcl::extend(prevVals[2]);
            X.store_partial(lastCols, pDXCol);

            prevVals[0].load_partial(lastCols,pImgCol[0] - 1); nextVals[0].load_partial(lastCols, pImgCol[2] - 1);
            prevVals[1].load_partial(lastCols, pImgCol[0]); nextVals[1].load_partial(lastCols, pImgCol[2]);
            prevVals[2].load_partial(lastCols, pImgCol[0] + 1); nextVals[2].load_partial(lastCols, pImgCol[2] + 1);

            vcl::Vec16s Y = vcl::extend(nextVals[0]) - vcl::extend(prevVals[0]);
            Y = Y + 2 * (vcl::extend(nextVals[1]) - vcl::extend(prevVals[1]));
            Y = Y + vcl::extend(nextVals[2]) - vcl::extend(prevVals[2]);
            Y.store_partial(lastCols, pDYCol);
        }

        pImgRow[0] += imgStep;
        pImgRow[1] += imgStep;
        pImgRow[2] += imgStep;
        pDXRow += dxStep;
        pDYRow += dyStep;
    }
}

void SpamGradient::Simple(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
    dx.create(img.rows, img.cols, CV_16SC1);
    dy.create(img.rows, img.cols, CV_16SC1);
    dx = 0;
    dy = 0;

    if (img.rows<3 || img.cols<3)
    {
        return;
    }

    const int rowBeg = std::max(1, roi.y);
    const int colBeg = std::max(1, roi.x);
    const int rowEnd = std::min(img.rows-1, roi.y + roi.height);
    const int colEnd = std::min(img.cols-1, roi.x + roi.width);

    SimpleGradient sg(&img, &dx, &dy, colBeg, colEnd);
    tbb::parallel_for(tbb::blocked_range<int>(rowBeg, rowEnd), sg);
}

void SpamGradient::SimpleNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
    dx.create(img.rows, img.cols, CV_32FC1);
    dy.create(img.rows, img.cols, CV_32FC1);
    dx = 0;
    dy = 0;

    if (img.rows < 3 || img.cols < 3)
    {
        return;
    }

    const int rowBeg = std::max(1, roi.y);
    const int colBeg = std::max(1, roi.x);
    const int rowEnd = std::min(img.rows - 1, roi.y + roi.height);
    const int colEnd = std::min(img.cols - 1, roi.x + roi.width);

    SimpleNormGradient sng(&img, &dx, &dy, colBeg, colEnd);
    tbb::parallel_for(tbb::blocked_range<int>(rowBeg, rowEnd), sng);
}

void SpamGradient::Sobel(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
    dx.create(img.rows, img.cols, CV_16SC1);
    dy.create(img.rows, img.cols, CV_16SC1);
    dx = 0;
    dy = 0;

    if (img.rows < 3 || img.cols < 3)
    {
        return;
    }

    const int rowBeg = std::max(1, roi.y);
    const int colBeg = std::max(1, roi.x);
    const int rowEnd = std::min(img.rows - 1, roi.y + roi.height);
    const int colEnd = std::min(img.cols - 1, roi.x + roi.width);

    SobelGradient sg(&img, &dx, &dy, colBeg, colEnd);
    tbb::parallel_for(tbb::blocked_range<int>(rowBeg, rowEnd), sg);
}

void SpamGradient::SobelNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi)
{
}