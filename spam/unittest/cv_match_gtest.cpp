#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>

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
}