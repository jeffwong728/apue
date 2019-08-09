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