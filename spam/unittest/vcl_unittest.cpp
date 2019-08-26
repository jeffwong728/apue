#define BOOST_TEST_MODULE TestVcl
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <vectorclass/instrset.h>
#include <vectorclass/vectorclass.h>
#include <opencv2/highgui.hpp>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

struct TestVclConfig 
{
    TestVclConfig()
    {
        std::cout << "Global setup" << std::endl; 
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
        std::cout << "Vcl detected instruction set: "<< vcl::instrset_detect() << std::endl;
    }

    ~TestVclConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_GLOBAL_FIXTURE(TestVclConfig);

BOOST_AUTO_TEST_CASE(test_Vcl_Instruction_Set)
{
    BOOST_CHECK_GE(vcl::instrset_detect(), 8);
}

BOOST_AUTO_TEST_CASE(test_Vcl_Horizontal_Add)
{
    int32_t data[]{1, 2, 3, 4, 5, 6, 7, 8};
    vcl::Vec8i a;
    a.load(data);
    BOOST_CHECK_EQUAL(vcl::horizontal_add(a), 36);
}

BOOST_AUTO_TEST_CASE(test_Vcl_Approx_Rsqrt)
{
    vcl::Vec8f a{1.f, 2.f, 3.f, 10000.f, 20000.f, 50000.f, 80000.f, 120000.f};
    vcl::Vec8f b = vcl::approx_rsqrt(a);
    BOOST_CHECK_CLOSE(b[0], 1 / std::sqrt(1.f), 1e-1);
    BOOST_CHECK_CLOSE(b[1], 1 / std::sqrt(2.f), 1e-1);
    BOOST_CHECK_CLOSE(b[2], 1 / std::sqrt(3.f), 1e-1);
    BOOST_CHECK_CLOSE(b[3], 1 / std::sqrt(10000.f), 1e-1);
    BOOST_CHECK_CLOSE(b[4], 1 / std::sqrt(20000.f), 1e-1);
    BOOST_CHECK_CLOSE(b[5], 1 / std::sqrt(50000.f), 1e-1);
    BOOST_CHECK_CLOSE(b[6], 1 / std::sqrt(80000.f), 1e-1);
    BOOST_CHECK_CLOSE(b[7], 1 / std::sqrt(120000.f), 1e-1);
}