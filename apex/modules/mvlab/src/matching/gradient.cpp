#include "precomp.hpp"
#include "gradient.h"

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
    static void kernel(const vcl::Vec16uc(&pixelVals)[12], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1, int16_t *pD);
    static void kernelPartial(const vcl::Vec16uc(&pixelVals)[12], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1, const int lastCols, int16_t *pD);
    static void kernel(const vcl::Vec16uc(&pixelVals)[9], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1, int16_t *pD);
    static void kernelPartial(const vcl::Vec16uc(&pixelVals)[9], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1, const int lastCols, int16_t *pD);

    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

struct SimpleSobelGradient
{
    SimpleSobelGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd)
        : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd) {}
    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
};

struct SobelNormGradient
{
    SobelNormGradient(const cv::Mat *const i, cv::Mat *const x, cv::Mat *const y, const int cBeg, const int cEnd, const int mc)
        : img(i), dx(x), dy(y), colBeg(cBeg), colEnd(cEnd), minContrast(mc) {}

    static vcl::Vec16s kernel(const vcl::Vec16uc(&pixelVals)[9], const int i0, const int i1, const int j0, const int j1, const int k0, const int k1);
    static void normalize(const vcl::Vec8i &X, const vcl::Vec8i &Y, const vcl::Vec8i &minContrast, float *xNorm, float *yNorm);
    static void normalizePartial(const vcl::Vec8i &X, const vcl::Vec8i &Y, const vcl::Vec8i &minContrast, const int lastCols, float *xNorm, float *yNorm);
    void operator()(const tbb::blocked_range<int>& br) const;
    const int colBeg;
    const int colEnd;
    cv::Mat *const dx;
    cv::Mat *const dy;
    const cv::Mat *const img;
    static const vcl::Vec8i ones8i;
    static const vcl::Vec8i zeros8i;
    const int minContrast;
};

const vcl::Vec8i SobelNormGradient::ones8i{ 1 };
const vcl::Vec8i SobelNormGradient::zeros8i{ 0 };

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

inline void SobelGradient::kernel(const vcl::Vec16uc(&pixelVals)[12],
    const int i0, const int i1,
    const int j0, const int j1,
    const int k0, const int k1,
    int16_t *pD)
{
    vcl::Vec16s X = vcl::extend(pixelVals[i1]) - vcl::extend(pixelVals[i0]);
    X = X + 2 * (vcl::extend(pixelVals[j1]) - vcl::extend(pixelVals[j0]));
    X = X + vcl::extend(pixelVals[k1]) - vcl::extend(pixelVals[k0]);
    X.store(pD);
}

inline void SobelGradient::kernelPartial(const vcl::Vec16uc(&pixelVals)[12],
    const int i0, const int i1,
    const int j0, const int j1,
    const int k0, const int k1,
    const int lastCols,
    int16_t *pD)
{
    vcl::Vec16s X = vcl::extend(pixelVals[i1]) - vcl::extend(pixelVals[i0]);
    X = X + 2 * (vcl::extend(pixelVals[j1]) - vcl::extend(pixelVals[j0]));
    X = X + vcl::extend(pixelVals[k1]) - vcl::extend(pixelVals[k0]);
    X.store_partial(lastCols, pD);
}

inline void SobelGradient::kernel(const vcl::Vec16uc(&pixelVals)[9],
    const int i0, const int i1,
    const int j0, const int j1,
    const int k0, const int k1,
    int16_t *pD)
{
    vcl::Vec16s X = vcl::extend(pixelVals[i1]) - vcl::extend(pixelVals[i0]);
    X = X + 2 * (vcl::extend(pixelVals[j1]) - vcl::extend(pixelVals[j0]));
    X = X + vcl::extend(pixelVals[k1]) - vcl::extend(pixelVals[k0]);
    X >>= 3;
    X.store(pD);
}

inline void SobelGradient::kernelPartial(const vcl::Vec16uc(&pixelVals)[9],
    const int i0, const int i1,
    const int j0, const int j1,
    const int k0, const int k1,
    const int lastCols,
    int16_t *pD)
{
    vcl::Vec16s X = vcl::extend(pixelVals[i1]) - vcl::extend(pixelVals[i0]);
    X = X + 2 * (vcl::extend(pixelVals[j1]) - vcl::extend(pixelVals[j0]));
    X = X + vcl::extend(pixelVals[k1]) - vcl::extend(pixelVals[k0]);
    X >>= 3;
    X.store_partial(lastCols, pD);
}

void SobelGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    const int width = colEnd - colBeg;
    const int height = rowEnd - rowBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularHeight = height & (-2);
    const int regularColEnd = colBeg + regularWidth;
    const int regularRowEnd = rowBeg + regularHeight;

    const auto imgStep = img->step1();
    const auto dxStep = dx->step1();
    const auto dyStep = dy->step1();
    const uint8_t *pImgRow[3]{ img->ptr<uint8_t>(rowBeg - 1) + 1, img->ptr<uint8_t>(rowBeg) + 1, img->ptr<uint8_t>(rowBeg + 1) + 1 };
    int16_t *pDXRow = dx->ptr<int16_t>(rowBeg) + 1;
    int16_t *pDYRow = dy->ptr<int16_t>(rowBeg) + 1;
    vcl::Vec16uc pixelVals[12];

    for (int row = rowBeg; row < regularRowEnd; row += 2)
    {
        int16_t *pDXCol = pDXRow;
        int16_t *pDYCol = pDYRow;
        const uint8_t *pImgCol[4] = { pImgRow[0], pImgRow[1], pImgRow[2], pImgRow[2] + imgStep };

        for (int col = colBeg; col < regularColEnd; col += vectorSize)
        {
            pixelVals[0].load(pImgCol[0] - 1); pixelVals[1].load(pImgCol[0]); pixelVals[2].load(pImgCol[0] + 1);
            pixelVals[3].load(pImgCol[1] - 1); pixelVals[4].load(pImgCol[1]); pixelVals[5].load(pImgCol[1] + 1);
            pixelVals[6].load(pImgCol[2] - 1); pixelVals[7].load(pImgCol[2]); pixelVals[8].load(pImgCol[2] + 1);
            pixelVals[9].load(pImgCol[3] - 1); pixelVals[10].load(pImgCol[3]); pixelVals[11].load(pImgCol[3] + 1);

            kernel(pixelVals, 0, 2, 3, 5, 6, 8, pDXCol);
            kernel(pixelVals, 0, 6, 1, 7, 2, 8, pDYCol);
            kernel(pixelVals, 3, 5, 6, 8, 9, 11, pDXCol + dxStep);
            kernel(pixelVals, 3, 9, 4, 10, 5, 11, pDYCol + dyStep);

            pImgCol[0] += vectorSize;
            pImgCol[1] += vectorSize;
            pImgCol[2] += vectorSize;
            pImgCol[3] += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;

            pixelVals[0].load_partial(lastCols, pImgCol[0] - 1); pixelVals[1].load_partial(lastCols, pImgCol[0]); pixelVals[2].load_partial(lastCols, pImgCol[0] + 1);
            pixelVals[3].load_partial(lastCols, pImgCol[1] - 1); pixelVals[4].load_partial(lastCols, pImgCol[1]); pixelVals[5].load_partial(lastCols, pImgCol[1] + 1);
            pixelVals[6].load_partial(lastCols, pImgCol[2] - 1); pixelVals[7].load_partial(lastCols, pImgCol[2]); pixelVals[8].load_partial(lastCols, pImgCol[2] + 1);
            pixelVals[9].load_partial(lastCols, pImgCol[3] - 1); pixelVals[10].load_partial(lastCols, pImgCol[3]); pixelVals[11].load_partial(lastCols, pImgCol[3] + 1);

            kernelPartial(pixelVals, 0, 2, 3, 5, 6, 8, lastCols, pDXCol);
            kernelPartial(pixelVals, 0, 6, 1, 7, 2, 8, lastCols, pDYCol);
            kernelPartial(pixelVals, 3, 5, 6, 8, 9, 11, lastCols, pDXCol + dxStep);
            kernelPartial(pixelVals, 3, 9, 4, 10, 5, 11, lastCols, pDYCol + dyStep);
        }

        pImgRow[0] += imgStep*2;
        pImgRow[1] += imgStep*2;
        pImgRow[2] += imgStep*2;
        pDXRow += dxStep*2;
        pDYRow += dyStep*2;
    }

    if (height > regularHeight)
    {
        int16_t *pDXCol = pDXRow;
        int16_t *pDYCol = pDYRow;
        const uint8_t *pImgCol[3] = { pImgRow[0], pImgRow[1], pImgRow[2]};

        for (int col = colBeg; col < regularColEnd; col += vectorSize)
        {
            pixelVals[0].load(pImgCol[0] - 1); pixelVals[1].load(pImgCol[0]); pixelVals[2].load(pImgCol[0] + 1);
            pixelVals[3].load(pImgCol[1] - 1); pixelVals[5].load(pImgCol[1] + 1);
            pixelVals[6].load(pImgCol[2] - 1); pixelVals[7].load(pImgCol[2]); pixelVals[8].load(pImgCol[2] + 1);

            kernel(pixelVals, 0, 2, 3, 5, 6, 8, pDXCol);
            kernel(pixelVals, 0, 6, 1, 7, 2, 8, pDYCol);

            pImgCol[0] += vectorSize;
            pImgCol[1] += vectorSize;
            pImgCol[2] += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;
            pixelVals[0].load_partial(lastCols, pImgCol[0] - 1); pixelVals[1].load_partial(lastCols, pImgCol[0]); pixelVals[2].load_partial(lastCols, pImgCol[0] + 1);
            pixelVals[3].load_partial(lastCols, pImgCol[1] - 1); pixelVals[5].load_partial(lastCols, pImgCol[1] + 1);
            pixelVals[6].load_partial(lastCols, pImgCol[2] - 1); pixelVals[7].load_partial(lastCols, pImgCol[2]); pixelVals[8].load_partial(lastCols, pImgCol[2] + 1);

            kernelPartial(pixelVals, 0, 2, 3, 5, 6, 8, lastCols, pDXCol);
            kernelPartial(pixelVals, 0, 6, 1, 7, 2, 8, lastCols, pDYCol);
        }
    }
}

void SimpleSobelGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    const int width = colEnd - colBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularColEnd = colBeg + regularWidth;

    const auto imgStep = img->step1();
    const auto dxStep = dx->step1();
    const auto dyStep = dy->step1();
    const uint8_t *pImgRow[3]{ img->ptr<uint8_t>(rowBeg - 1) + colBeg, img->ptr<uint8_t>(rowBeg) + colBeg, img->ptr<uint8_t>(rowBeg + 1) + colBeg };
    int16_t *pDXRow = dx->ptr<int16_t>(rowBeg) + colBeg;
    int16_t *pDYRow = dy->ptr<int16_t>(rowBeg) + colBeg;
    vcl::Vec16uc pixelVals[9];

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        int16_t *pDXCol = pDXRow;
        int16_t *pDYCol = pDYRow;
        const uint8_t *pImgCol[4] = { pImgRow[0], pImgRow[1], pImgRow[2] };

        for (int col = colBeg; col < regularColEnd; col += vectorSize)
        {
            pixelVals[0].load(pImgCol[0] - 1); pixelVals[1].load(pImgCol[0]); pixelVals[2].load(pImgCol[0] + 1);
            pixelVals[3].load(pImgCol[1] - 1); pixelVals[5].load(pImgCol[1] + 1);
            pixelVals[6].load(pImgCol[2] - 1); pixelVals[7].load(pImgCol[2]); pixelVals[8].load(pImgCol[2] + 1);

            SobelGradient::kernel(pixelVals, 0, 2, 3, 5, 6, 8, pDXCol);
            SobelGradient::kernel(pixelVals, 0, 6, 1, 7, 2, 8, pDYCol);

            pImgCol[0] += vectorSize;
            pImgCol[1] += vectorSize;
            pImgCol[2] += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;

            pixelVals[0].load_partial(lastCols, pImgCol[0] - 1); pixelVals[1].load_partial(lastCols, pImgCol[0]); pixelVals[2].load_partial(lastCols, pImgCol[0] + 1);
            pixelVals[3].load_partial(lastCols, pImgCol[1] - 1); pixelVals[5].load_partial(lastCols, pImgCol[1] + 1);
            pixelVals[6].load_partial(lastCols, pImgCol[2] - 1); pixelVals[7].load_partial(lastCols, pImgCol[2]); pixelVals[8].load_partial(lastCols, pImgCol[2] + 1);

            SobelGradient::kernelPartial(pixelVals, 0, 2, 3, 5, 6, 8, lastCols, pDXCol);
            SobelGradient::kernelPartial(pixelVals, 0, 6, 1, 7, 2, 8, lastCols, pDYCol);
        }

        pImgRow[0] += imgStep;
        pImgRow[1] += imgStep;
        pImgRow[2] += imgStep;
        pDXRow += dxStep;
        pDYRow += dyStep;
    }
}

inline vcl::Vec16s SobelNormGradient::kernel(const vcl::Vec16uc(&pixelVals)[9],
    const int i0, const int i1,
    const int j0, const int j1,
    const int k0, const int k1)
{
    vcl::Vec16s X = vcl::extend(pixelVals[i1]) - vcl::extend(pixelVals[i0]);
    X = X + 2 * (vcl::extend(pixelVals[j1]) - vcl::extend(pixelVals[j0]));
    X = X + vcl::extend(pixelVals[k1]) - vcl::extend(pixelVals[k0]);

    return X;
}

inline void SobelNormGradient::normalize(const vcl::Vec8i &X, const vcl::Vec8i &Y,
    const vcl::Vec8i &minContrast, float *xNorm, float *yNorm)
{
    vcl::Vec8i sqrSum = X * X + Y * Y;
    vcl::Vec8i normSqrSum = vcl::select(sqrSum == 0, ones8i, sqrSum);
    vcl::Vec8i supress = sqrSum < minContrast;
    vcl::Vec8i supressX = vcl::select(supress, zeros8i, X);
    vcl::Vec8i supressY = vcl::select(supress, zeros8i, Y);
    vcl::Vec8f rSqrt = vcl::approx_rsqrt(vcl::to_float(normSqrSum));
    vcl::Vec8f NX = vcl::to_float(supressX) * rSqrt;
    vcl::Vec8f NY = vcl::to_float(supressY) * rSqrt;
    NX.store(xNorm);
    NY.store(yNorm);
}

inline void SobelNormGradient::normalizePartial(const vcl::Vec8i &X, const vcl::Vec8i &Y,
    const vcl::Vec8i &minContrast, const int lastCols, float *xNorm, float *yNorm)
{
    vcl::Vec8i sqrSum = X * X + Y * Y;
    vcl::Vec8i normSqrSum = vcl::select(sqrSum == 0, ones8i, sqrSum);
    vcl::Vec8i supress = sqrSum < minContrast;
    vcl::Vec8i supressX = vcl::select(supress, zeros8i, X);
    vcl::Vec8i supressY = vcl::select(supress, zeros8i, Y);
    vcl::Vec8f rSqrt = vcl::approx_rsqrt(vcl::to_float(normSqrSum));
    vcl::Vec8f NX = vcl::to_float(supressX) * rSqrt;
    vcl::Vec8f NY = vcl::to_float(supressY) * rSqrt;
    NX.store_partial(lastCols, xNorm);
    NY.store_partial(lastCols, yNorm);
}

void SobelNormGradient::operator()(const tbb::blocked_range<int>& br) const
{
    const int rowBeg = br.begin();
    const int rowEnd = br.end();

    constexpr int vectorSize = 16;
    constexpr int halfVectorSize = vectorSize / 2;
    const int width = colEnd - colBeg;
    const int regularWidth = width & (-vectorSize);
    const int regularColEnd = colBeg + regularWidth;

    const auto imgStep = img->step1();
    const auto dxStep = dx->step1();
    const auto dyStep = dy->step1();
    const uint8_t *pImgRow[3]{ img->ptr<uint8_t>(rowBeg - 1) + colBeg, img->ptr<uint8_t>(rowBeg) + colBeg, img->ptr<uint8_t>(rowBeg + 1) + colBeg };
    float *pDXRow = dx->ptr<float>(rowBeg) + colBeg;
    float *pDYRow = dy->ptr<float>(rowBeg) + colBeg;
    vcl::Vec16uc pixelVals[9];
    vcl::Vec8i vecMinContrast(minContrast*minContrast *64);

    for (int row = rowBeg; row < rowEnd; ++row)
    {
        float *pDXCol = pDXRow;
        float *pDYCol = pDYRow;
        const uint8_t *pImgCol[4] = { pImgRow[0], pImgRow[1], pImgRow[2] };

        for (int col = colBeg; col < regularColEnd; col += vectorSize)
        {
            pixelVals[0].load(pImgCol[0] - 1); pixelVals[1].load(pImgCol[0]); pixelVals[2].load(pImgCol[0] + 1);
            pixelVals[3].load(pImgCol[1] - 1); pixelVals[5].load(pImgCol[1] + 1);
            pixelVals[6].load(pImgCol[2] - 1); pixelVals[7].load(pImgCol[2]); pixelVals[8].load(pImgCol[2] + 1);

            vcl::Vec16s X = kernel(pixelVals, 0, 2, 3, 5, 6, 8);
            vcl::Vec16s Y = kernel(pixelVals, 0, 6, 1, 7, 2, 8);
            vcl::Vec8i  lowX = vcl::extend(X.get_low()), highX = vcl::extend(X.get_high());
            vcl::Vec8i  lowY = vcl::extend(Y.get_low()), highY = vcl::extend(Y.get_high());

            normalize(lowX, lowY, vecMinContrast, pDXCol, pDYCol);
            normalize(highX, highY, vecMinContrast, pDXCol + halfVectorSize, pDYCol + halfVectorSize);

            pImgCol[0] += vectorSize;
            pImgCol[1] += vectorSize;
            pImgCol[2] += vectorSize;
            pDXCol += vectorSize;
            pDYCol += vectorSize;
        }

        if (width > regularWidth)
        {
            const int lastCols = width - regularWidth;
            pixelVals[0].load_partial(lastCols, pImgCol[0] - 1); pixelVals[1].load_partial(lastCols, pImgCol[0]); pixelVals[2].load_partial(lastCols, pImgCol[0] + 1);
            pixelVals[3].load_partial(lastCols, pImgCol[1] - 1); pixelVals[5].load_partial(lastCols, pImgCol[1] + 1);
            pixelVals[6].load_partial(lastCols, pImgCol[2] - 1); pixelVals[7].load_partial(lastCols, pImgCol[2]); pixelVals[8].load_partial(lastCols, pImgCol[2] + 1);

            vcl::Vec16s X = kernel(pixelVals, 0, 2, 3, 5, 6, 8);
            vcl::Vec16s Y = kernel(pixelVals, 0, 6, 1, 7, 2, 8);

            vcl::Vec8i  lowX = vcl::extend(X.get_low());
            vcl::Vec8i  lowY = vcl::extend(Y.get_low());

            if (lastCols == halfVectorSize)
            {
                normalize(lowX, lowY, vecMinContrast, pDXCol, pDYCol);
            }
            else if (lastCols < halfVectorSize)
            {
                normalizePartial(lowX, lowY, vecMinContrast, lastCols, pDXCol, pDYCol);
            }
            else
            {
                vcl::Vec8i  highX = vcl::extend(X.get_high());
                vcl::Vec8i  highY = vcl::extend(Y.get_high());
                normalize(lowX, lowY, vecMinContrast, pDXCol, pDYCol);
                normalizePartial(highX, highY, vecMinContrast, lastCols - halfVectorSize, pDXCol + halfVectorSize, pDYCol + halfVectorSize);
            }
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

    SimpleSobelGradient ssg(&img, &dx, &dy, colBeg, colEnd);
    ssg(tbb::blocked_range<int>(rowBeg, rowEnd));
}

void SpamGradient::SobelNormalize(const cv::Mat &img, cv::Mat &dx, cv::Mat &dy, const cv::Rect &roi, const int minContrast)
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

    SobelNormGradient sng(&img, &dx, &dy, colBeg, colEnd, minContrast);
    sng(tbb::blocked_range<int>(rowBeg, rowEnd));
}