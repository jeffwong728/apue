#include <opencv2/highgui.hpp>
#include <opencv2/mvlab.hpp>
#include <gtest/gtest.h>

namespace
{
TEST(RegionTest, EmptyRegion)
{
    cv::Ptr<cv::mvlab::Region> rgn = cv::mvlab::Region::GenEmpty();
    EXPECT_TRUE(rgn);
    EXPECT_TRUE(rgn->Empty());
    EXPECT_EQ(rgn->Area(), 0.);
    EXPECT_EQ(rgn->Count(), 1);
    EXPECT_EQ(rgn->CountRuns(), 0);
    EXPECT_EQ(rgn->CountRows(), 0);
    EXPECT_EQ(rgn->CountConnect(), 0);
    EXPECT_EQ(rgn->CountHoles(), 0);
    EXPECT_EQ(rgn->Centroid(), cv::Point2d());
}

TEST(RegionTest, OnePointRegion)
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