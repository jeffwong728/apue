#define BOOST_TEST_MODULE TestSimdPerf
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <opencv2/highgui.hpp>
#include <Simd/SimdLib.h>
#include <Simd/SimdBase.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>

struct TestSimdPerfConfig
{
    TestSimdPerfConfig()
    {
        std::cout << "Global setup" << std::endl;
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
    }

    ~TestSimdPerfConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_AUTO_TEST_CASE(test_Simd_Reduce2X2_Performance)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat dst(grayImg.size() / 2, grayImg.type());
    ::SimdReduceGray2x2(grayImg.data, grayImg.cols, grayImg.rows, grayImg.step1(), dst.data, dst.cols, dst.rows, dst.step1());
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Simd Reduce2X2 spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(dst.rows, grayImg.rows / 2);

    UnitTestHelper::WriteImage(dst, std::string("mista_Simd_Reduce2X2.png"));
}