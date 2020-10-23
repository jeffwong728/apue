#include <opencv2/highgui.hpp>
#include <opencv2/mvlab.hpp>
#include <gtest/gtest.h>
#include <tbb/tick_count.h>
#include "helper.h"

namespace
{
  TEST(RegionMiscTest, CountResistor)
  {
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(UnitTestHelper::GetFullPath(std::string("mix-cs8svf-pcb.jpg")));
    EXPECT_FALSE(grayImg.empty());

    cv::Ptr<cv::mvlab::Region> rgn = cv::mvlab::Threshold(grayImg, 128, 255);
    EXPECT_TRUE(rgn);

    cv::Ptr<cv::mvlab::Region> se = cv::mvlab::Region::GenStructuringElement("circle", 5);
    EXPECT_FALSE(se->Empty());

    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Ptr<cv::mvlab::Region> ergn = rgn->Erosion(se);
    tbb::tick_count t2 = tbb::tick_count::now();
    EXPECT_LT(ergn->Area(), rgn->Area());
    GTEST_TIME_COUT << (t2 - t1).seconds() * 1000 << "ms" << std::endl;
  }
}