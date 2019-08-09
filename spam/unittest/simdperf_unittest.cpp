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
        int info = SimdCpuInfo();
        std::cout << "SSE: " << (info&(1 << SimdCpuInfoSse) ? "Yes" : "No") << std::endl;
        std::cout << "SSE2: " << (info&(1 << SimdCpuInfoSse2) ? "Yes" : "No") << std::endl;
        std::cout << "SSE3: " << (info&(1 << SimdCpuInfoSse3) ? "Yes" : "No") << std::endl;
        std::cout << "SSSE3: " << (info&(1 << SimdCpuInfoSsse3) ? "Yes" : "No") << std::endl;
        std::cout << "SSE4.1: " << (info&(1 << SimdCpuInfoSse41) ? "Yes" : "No") << std::endl;
        std::cout << "SSE4.2: " << (info&(1 << SimdCpuInfoSse42) ? "Yes" : "No") << std::endl;
        std::cout << "AVX: " << (info&(1 << SimdCpuInfoAvx) ? "Yes" : "No") << std::endl;
        std::cout << "AVX2: " << (info&(1 << SimdCpuInfoAvx2) ? "Yes" : "No") << std::endl;
        std::cout << "AVX-512F: " << (info&(1 << SimdCpuInfoAvx512f) ? "Yes" : "No") << std::endl;
        std::cout << "AVX-512BW: " << (info&(1 << SimdCpuInfoAvx512bw) ? "Yes" : "No") << std::endl;
        std::cout << "PowerPC-Altivec: " << (info&(1 << SimdCpuInfoVmx) ? "Yes" : "No") << std::endl;
        std::cout << "PowerPC-VSX: " << (info&(1 << SimdCpuInfoVsx) ? "Yes" : "No") << std::endl;
        std::cout << "ARM-NEON: " << (info&(1 << SimdCpuInfoNeon) ? "Yes" : "No") << std::endl;
        std::cout << "MIPS-MSA: " << (info&(1 << SimdCpuInfoMsa) ? "Yes" : "No") << std::endl;
    }

    ~TestSimdPerfConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_GLOBAL_FIXTURE(TestSimdPerfConfig);

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