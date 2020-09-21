#include "precomp.hpp"
#include "basic.h"

struct CVPointLesser
{
    bool operator()(const cv::Point &a, const cv::Point &b) const
    {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    }
};

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

void BasicImgProc::TrackCurves(const ScalablePointSequence &points,
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