#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>
#include <Simd/SimdLib.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>

namespace
{
TEST(SimdTest, Reduce2X2)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat dst(grayImg.size()/2, grayImg.type());
    ::SimdReduceGray2x2(grayImg.data, grayImg.cols, grayImg.rows, grayImg.step1(), dst.data, dst.cols, dst.rows, dst.step1());
    tbb::tick_count t2 = tbb::tick_count::now();

    EXPECT_EQ(dst.rows, grayImg.rows/2);
}
}