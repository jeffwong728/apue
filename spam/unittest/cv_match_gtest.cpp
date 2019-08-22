#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>
#include <random>
#include <algorithm>
#include <vectorclass/vectorclass.h>
constexpr int simdSize = 8;

double calcValue1(const int64_t T, const int64_t S, const int64_t n)
{
    int64_t A = n * T;
    int64_t B = S * S;
    if (A <= B)
    {
        return -1.0;
    }
    else
    {
        return std::sqrt(static_cast<double>(A - B)) / std::sqrt(static_cast<double>(n));
    }
}

void sequenceSum(std::vector<uint32_t> &vec, int64_t &sum, int64 &sqrSum)
{
    vec.resize((static_cast<int>(vec.size()) + simdSize - 1) & (-simdSize), 0);

    vcl::Vec8ui sumVec(0);
    vcl::Vec8ui sqrSumVec(0);
    const uint32_t *pVals = vec.data();
    const int numItems = static_cast<int>(vec.size());
    for (int n = 0; n < numItems; n += simdSize)
    {
        vcl::Vec8ui tempVec;
        tempVec.load(pVals);
        sumVec += tempVec;
        sqrSumVec += (tempVec * tempVec);
        pVals += 8;
    }

    sum = vcl::horizontal_add(sumVec);
    sqrSum = vcl::horizontal_add(sqrSumVec);
}

int64_t sequenceDot(std::vector<uint32_t> &vec1, std::vector<uint32_t> &vec2)
{
    vec1.resize((static_cast<int>(vec1.size()) + simdSize - 1) & (-simdSize), 0);
    vec2.resize((static_cast<int>(vec2.size()) + simdSize - 1) & (-simdSize), 0);

    vcl::Vec8ui sumVec(0);
    const uint32_t *pVals1 = vec1.data();
    const uint32_t *pVals2 = vec2.data();
    const int numItems = static_cast<int>(vec1.size());
    for (int n = 0; n < numItems; n += simdSize)
    {
        vcl::Vec8ui tempVec1, tempVec2;
        tempVec1.load(pVals1);
        tempVec2.load(pVals2);
        sumVec += (tempVec1 * tempVec2);
        pVals1 += 8;
        pVals2 += 8;
    }

    return vcl::horizontal_add(sumVec);
}

namespace {
TEST(CVFloorTest, Positive)
{
    EXPECT_EQ(1, cvFloor(1.0));
    EXPECT_EQ(1, cvFloor(1.1));
    EXPECT_EQ(1, cvFloor(1.3));
    EXPECT_EQ(1, cvFloor(1.4));
    EXPECT_EQ(1, cvFloor(1.5));
    EXPECT_EQ(1, cvFloor(1.7));
    EXPECT_EQ(1, cvFloor(1.9));
}

TEST(CVFloorTest, Zero)
{
    EXPECT_EQ(0, cvFloor(0.0));
}

TEST(CVFloorTest, Negative)
{
    EXPECT_EQ(-1, cvFloor(-1.0));
    EXPECT_EQ(-2, cvFloor(-1.1));
    EXPECT_EQ(-2, cvFloor(-1.3));
    EXPECT_EQ(-2, cvFloor(-1.4));
    EXPECT_EQ(-2, cvFloor(-1.5));
    EXPECT_EQ(-2, cvFloor(-1.7));
    EXPECT_EQ(-2, cvFloor(-1.9));
}

TEST(CVCeilTest, Positive)
{
    EXPECT_EQ(1, cvCeil(1.0));
    EXPECT_EQ(2, cvCeil(1.1));
    EXPECT_EQ(2, cvCeil(1.3));
    EXPECT_EQ(2, cvCeil(1.4));
    EXPECT_EQ(2, cvCeil(1.5));
    EXPECT_EQ(2, cvCeil(1.7));
    EXPECT_EQ(2, cvCeil(1.9));
}

TEST(CVCeilTest, Zero)
{
    EXPECT_EQ(0, cvCeil(0.0));
}

TEST(CVCeilTest, Negative)
{
    EXPECT_EQ(-1, cvCeil(-1.0));
    EXPECT_EQ(-1, cvCeil(-1.1));
    EXPECT_EQ(-1, cvCeil(-1.3));
    EXPECT_EQ(-1, cvCeil(-1.4));
    EXPECT_EQ(-1, cvCeil(-1.5));
    EXPECT_EQ(-1, cvCeil(-1.7));
    EXPECT_EQ(-1, cvCeil(-1.9));
}

TEST(CVImageRotateTest, Dot)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Point center{ 50, 30 };
    img.at<uint8_t>(center) = 0xFF;
    UnitTestHelper::WriteImage(img, "rotate_dot_src.png");

    cv::Mat rotMat = cv::getRotationMatrix2D(center, 30, 1.0);
    cv::Mat dst;
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));
    UnitTestHelper::WriteImage(dst, "rotate_dot_dst.png");
}

TEST(CVImageRotateTest, Cross)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Point center{50, 30};
    cv::drawMarker(img, center, CV_RGB(255, 255, 255), cv::MARKER_CROSS, 10, 1);
    UnitTestHelper::WriteImage(img, "rotate_cross_src.png");

    cv::Mat rotMat = cv::getRotationMatrix2D(center, 30, 1.0);
    cv::Mat dst;
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));
    UnitTestHelper::WriteImage(dst, "rotate_cross_dst.png");
}

TEST(CVImageRotateTest, Rectangle)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Rect rect{ 30, 5, 30, 10 };
    cv::rectangle(img, rect, CV_RGB(255, 255, 255));
    UnitTestHelper::WriteImage(img, "rotate_rect_src.png");

    cv::Mat dst;
    cv::Mat rotMat = cv::getRotationMatrix2D((rect.tl() + rect.br())/2, 60, 1.0);
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));

    dst += img;
    UnitTestHelper::WriteImage(dst, "rotate_rect_dst.png");
}

TEST(CVImageTranslateTest, Rectangle)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Rect rect{ 30, 5, 30, 10 };
    cv::rectangle(img, rect, CV_RGB(255, 255, 255));
    UnitTestHelper::WriteImage(img, "translate_rect_src.png");

    cv::Mat transImg;
    cv::Mat transMat = (cv::Mat_<double>(2, 3) << 1, 0, -13, 0, 1, 14);
    cv::warpAffine(img, transImg, transMat, cv::Size(img.cols, img.rows));

    cv::Mat dst;
    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point(32, 24), 60, 1.0);
    cv::warpAffine(transImg, dst, rotMat, cv::Size(transImg.cols, transImg.rows));

    dst += transImg;
    UnitTestHelper::WriteImage(dst, "translate_rect_dst.png");
}

TEST(CVMatTest, Create)
{
    cv::Mat topImg;
    topImg.create(48, 64, CV_16SC1);
    topImg.create(48, 64, CV_16SC1);
    topImg = cv::Scalar(0);

    cv::Mat angImg;
    angImg.create(48, 64, CV_32F);
    angImg = cv::Scalar(0);

    EXPECT_EQ(topImg.rows, 48);
    EXPECT_EQ(topImg.cols, 64);
    EXPECT_EQ(topImg.elemSize1(), 2);
    EXPECT_EQ(topImg.step1(), 64);
    EXPECT_EQ(angImg.rows, 48);
    EXPECT_EQ(angImg.cols, 64);
    EXPECT_EQ(angImg.elemSize1(), 4);
    EXPECT_EQ(angImg.step1(), 64);
}

TEST(NCCFormulaTest, Equal)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part(tmpl);

    int64_t tmplSum=0, tmplSqrSum=0;
    int64_t partSum=0, partSqrSum=0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_DOUBLE_EQ(ncc, 1.0);
}

TEST(NCCFormulaTest, PositiveCorelation)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part;
    for (const uint32_t v : tmpl)
    {
        part.push_back(v*3+5);
    }

    int64_t tmplSum = 0, tmplSqrSum = 0;
    int64_t partSum = 0, partSqrSum = 0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_DOUBLE_EQ(ncc, 1.0);
}

TEST(NCCFormulaTest, NegtiveCorelation)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part;
    for (const uint32_t v : tmpl)
    {
        part.push_back(static_cast<uint32_t>(static_cast<int32_t>(v) * (-3) + 1000));
    }

    int64_t tmplSum = 0, tmplSqrSum = 0;
    int64_t partSum = 0, partSqrSum = 0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_NEAR(ncc, -1.0, 1e-9);
}

TEST(NCCFormulaTest, Zero)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part(100, 0);

    int64_t tmplSum = 0, tmplSqrSum = 0;
    int64_t partSum = 0, partSqrSum = 0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_DOUBLE_EQ(ncc, 0.0);
}

TEST(NCCFormulaTest, SameValue)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part(100, 9);

    int64_t tmplSum = 0, tmplSqrSum = 0;
    int64_t partSum = 0, partSqrSum = 0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_DOUBLE_EQ(ncc, 0.0);
}

TEST(NCCFormulaTest, RandomValue)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 100);

    std::vector<uint32_t> tmpl(100);
    std::generate(tmpl.begin(), tmpl.end(), [&]() { return dist(gen); });
    std::vector<uint32_t> part(100);
    std::generate(part.begin(), part.end(), [&]() { return dist(gen); });

    int64_t tmplSum = 0, tmplSqrSum = 0;
    int64_t partSum = 0, partSqrSum = 0;
    sequenceSum(tmpl, tmplSum, tmplSqrSum);
    sequenceSum(part, partSum, partSqrSum);

    double tmplVal1 = calcValue1(tmplSqrSum, tmplSum, 100);
    double partVal1 = calcValue1(partSqrSum, partSum, 100);

    double ncc = (sequenceDot(tmpl, part) - (tmplSum*partSum) / 100.0) / (tmplVal1*partVal1);
    EXPECT_LT(ncc, 1.0);
    EXPECT_GT(ncc, -1.0);
}

}