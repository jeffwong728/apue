#include <opencv2/highgui.hpp>
#include <opencv2/mvlab.hpp>
#include <gtest/gtest.h>
#include "helper.h"

namespace
{
  TEST(RegionMorphTest, Erosion)
  {
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(UnitTestHelper::GetFullPath(std::string("mista.png")));
    EXPECT_FALSE(grayImg.empty());

    cv::Ptr<cv::mvlab::Region> rgn;
    cv::mvlab::Threshold(grayImg, 128, 255, rgn);
    EXPECT_DOUBLE_EQ(rgn->Area(), 1932245.);

    cv::Ptr<cv::mvlab::Region> se = cv::mvlab::Region::GenStructuringElement("circle", 5);
    EXPECT_FALSE(se->Empty());

    cv::Ptr<cv::mvlab::Region> ergn = rgn->Erosion(se);
    EXPECT_LT(ergn->Area(), rgn->Area());
  }

  TEST(RegionMorphTest, Dilate)
  {
    cv::Ptr<cv::mvlab::Region> rgn = cv::mvlab::Region::GenRectangle(cv::Rect2f(3, 3, 1, 1));
    EXPECT_TRUE(rgn);
    EXPECT_FALSE(rgn->Empty());
    EXPECT_EQ(rgn->Area(), 1.);
    EXPECT_EQ(rgn->Count(), 1);
    EXPECT_EQ(rgn->CountRuns(), 1);
    EXPECT_EQ(rgn->CountRows(), 1);
    EXPECT_EQ(rgn->CountConnect(), 1);
    EXPECT_EQ(rgn->CountHoles(), 0);
    EXPECT_EQ(rgn->Centroid(), cv::Point2d(3, 3));
  }
}