#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>

int Factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }

    return result;
}

bool IsPrime(int n) {
    if (n <= 1) return false;
    if (n % 2 == 0) return n == 2;
    for (int i = 3; ; i += 2) {
        if (i > n / i) break;
        if (n % i == 0) return false;
    }
    return true;
}

namespace {
TEST(FactorialTest, Negative)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const cv::Rect roi(cv::Point(1972, 1836), cv::Point(2183, 2042));
    cv::Mat templ = grayImg(roi);

    int result_cols = grayImg.cols - templ.cols + 1;
    int result_rows = grayImg.rows - templ.rows + 1;

    cv::Mat result(result_rows, result_cols, CV_32FC1);
    cv::matchTemplate(grayImg, templ, result, cv::TM_CCORR_NORMED);

    EXPECT_EQ(1, Factorial(-5));
    EXPECT_EQ(1, Factorial(-1));
    EXPECT_GT(Factorial(-10), 0);
}

TEST(FactorialTest, Zero)
{
  EXPECT_EQ(1, Factorial(0));
}

TEST(FactorialTest, Positive)
{
  EXPECT_EQ(1, Factorial(1));
  EXPECT_EQ(2, Factorial(2));
  EXPECT_EQ(6, Factorial(3));
  EXPECT_EQ(40320, Factorial(8));
}

TEST(IsPrimeTest, Negative)
{
  EXPECT_FALSE(IsPrime(-1));
  EXPECT_FALSE(IsPrime(-2));
  EXPECT_FALSE(IsPrime(INT_MIN));
}

TEST(IsPrimeTest, Trivial)
{
  EXPECT_FALSE(IsPrime(0));
  EXPECT_FALSE(IsPrime(1));
  EXPECT_TRUE(IsPrime(2));
  EXPECT_TRUE(IsPrime(3));
}

TEST(IsPrimeTest, Positive)
{
  EXPECT_FALSE(IsPrime(4));
  EXPECT_TRUE(IsPrime(5));
  EXPECT_FALSE(IsPrime(6));
  EXPECT_TRUE(IsPrime(23));
}
}