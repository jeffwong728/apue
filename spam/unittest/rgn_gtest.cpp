#include "helper.h"
#include <cassert>
#include <opencv2/highgui.hpp>
#include <gtest/gtest.h>
#include <2geom/rect.h>
#include <2geom/circle.h>
#include <2geom/path-intersection.h>
#include <ui/proc/rgn.h>
#include <ui/proc/basic.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>

namespace
{
TEST(RgnAreaTest, Empty)
{
    SpamRgn rgn;
    EXPECT_DOUBLE_EQ(rgn.Area(), 0.0);
}

TEST(RgnAreaTest, SingleRun)
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 5);
    EXPECT_DOUBLE_EQ(rgn.Area(), 3.0);
}

TEST(RgnCentroidTest, Empty)
{
    SpamRgn rgn;
    EXPECT_DOUBLE_EQ(rgn.Centroid().x, 0.0);
    EXPECT_DOUBLE_EQ(rgn.Centroid().y, 0.0);
}

TEST(RgnCentroidTest, SingleRun)
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 5);
    EXPECT_DOUBLE_EQ(rgn.Centroid().x, 3.0);
    EXPECT_DOUBLE_EQ(rgn.Centroid().y, 1.0);
}

TEST(RgnCentroidTest, TwoRuns)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 2);
    rgn.AddRun(1, 0, 2);
    EXPECT_DOUBLE_EQ(rgn.Centroid().x, 0.5);
    EXPECT_DOUBLE_EQ(rgn.Centroid().y, 0.5);
}

TEST(RgnCentroidTest, MultipleRuns)
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 3);
    rgn.AddRun(2, 1, 4);
    rgn.AddRun(3, 0, 5);
    EXPECT_DOUBLE_EQ(rgn.Centroid().x, 2.0);
    EXPECT_DOUBLE_EQ(rgn.Centroid().y, 22.0/9);
}

TEST(PointInCircleTest, Positive)
{
    Geom::Path pth(Geom::Circle(Geom::Point(2, 2), 1));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(2, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(1, 2)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(2, 2)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(3, 2)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(2, 3)));
}

TEST(PointInCircleTest, Odd)
{
    Geom::Path pth(Geom::Circle(Geom::Point(3, 3), 1));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(3, 2)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(2, 3)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(3, 3)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(4, 3)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(3, 4)));
}

TEST(PointInCircleTest, Zero)
{
    Geom::Path pth(Geom::Circle(Geom::Point(0, 0), 1));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(0, -1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(-1, 0)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(0, 0)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(1, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(0, 1)));
}

TEST(PointInRectangleTest, Zero)
{
    Geom::Path pth(Geom::Rect(Geom::Point(0, 0), Geom::Point(1, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(0, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(0, 1)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, 1)));
}

TEST(PointInRectangleTest, Positive)
{
    Geom::Path pth(Geom::Rect(Geom::Point(1, 1), Geom::Point(3, 3)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(1, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(1, 2)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, 3)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(2, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(2, 2)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(2, 3)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(3, 1)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(3, 2)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(3, 3)));
}

TEST(PointInRectangleTest, Negative)
{
    Geom::Path pth(Geom::Rect(Geom::Point(-1, -1), Geom::Point(1, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(-1, -1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(-1, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(-1, 1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(0, -1)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(0, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(0, 1)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, -1)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, 0)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(1, 1)));
}

TEST(PointInRectangleTest, Big)
{
    Geom::Path pth(Geom::Rect(Geom::Point(-1, -1), Geom::Point(60, 57)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(60, -1)));
    EXPECT_FALSE(Geom::contains(pth, Geom::Point(60, 0)));
    EXPECT_TRUE(Geom::contains(pth, Geom::Point(60, 1)));
}

TEST(RgnCentroidTest, Circle)
{
    SpamRgn rgn0;
    rgn0.AddRun(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(0, 0), 50))));
    EXPECT_NEAR(rgn0.Centroid().x, 0.0, 0.01);
    EXPECT_NEAR(rgn0.Centroid().y, 0.0, 0.01);

    SpamRgn rgn1;
    rgn1.AddRun(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(2, 2), 50))));
    EXPECT_NEAR(rgn1.Centroid().x, 2.0, 0.01);
    EXPECT_NEAR(rgn1.Centroid().y, 2.0, 0.01);

    SpamRgn rgn2;
    rgn2.AddRun(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(-2, -2), 50))));
    EXPECT_NEAR(rgn2.Centroid().x, -2.0, 0.01);
    EXPECT_NEAR(rgn2.Centroid().y, -2.0, 0.01);

    SpamRgn rgn3;
    rgn3.AddRun(Geom::PathVector(Geom::Path(Geom::Circle(Geom::Point(10, 10), 50))));
    EXPECT_NEAR(rgn3.Centroid().x, 10.0, 0.01);
    EXPECT_NEAR(rgn3.Centroid().y, 10.0, 0.01);
}

TEST(RgnCentroidTest, Rectangle)
{
    SpamRgn rgn0;
    rgn0.AddRun(Geom::PathVector(Geom::Path(Geom::Rect(Geom::Point(0, 0), Geom::Point(1, 1)))));
    EXPECT_DOUBLE_EQ(rgn0.Centroid().x, 0.0);
    EXPECT_DOUBLE_EQ(rgn0.Centroid().y, 0.0);

    SpamRgn rgn1;
    rgn1.AddRun(Geom::PathVector(Geom::Path(Geom::Rect(Geom::Point(1, 1), Geom::Point(3, 3)))));
    EXPECT_DOUBLE_EQ(rgn1.Centroid().x, 1.5);
    EXPECT_DOUBLE_EQ(rgn1.Centroid().y, 1.5);

    SpamRgn rgn2;
    rgn2.AddRun(Geom::PathVector(Geom::Path(Geom::Rect(Geom::Point(-1, -1), Geom::Point(1, 1)))));
    EXPECT_DOUBLE_EQ(rgn2.Centroid().x, -0.5);
    EXPECT_DOUBLE_EQ(rgn2.Centroid().y, -0.5);

    SpamRgn rgn3;
    rgn3.AddRun(Geom::PathVector(Geom::Path(Geom::Rect(Geom::Point(-1, -2), Geom::Point(20, 10)))));
    EXPECT_DOUBLE_EQ(rgn3.Centroid().x, 9);
    EXPECT_DOUBLE_EQ(rgn3.Centroid().y, 3.5);
}

TEST(RgnPyramidTest, Circle)
{
    Geom::PathVector pv(Geom::Path(Geom::Circle(Geom::Point(100, 200), 50)));
    SpamRgn rgn0(pv*Geom::Scale(0.5, 0.5));

    cv::Point2d cg = SpamRgn(pv).Centroid();
    SpamRgn rgn1(pv * Geom::Translate(-cg.x, -cg.y)*Geom::Scale(0.5, 0.5)*Geom::Translate(cg.x*0.5, cg.y*0.5));

    EXPECT_DOUBLE_EQ(rgn0.Centroid().x, rgn1.Centroid().x);
    EXPECT_DOUBLE_EQ(rgn0.Centroid().y, rgn1.Centroid().y);
}

TEST(RgnPyramidTest, Rectangle)
{
    Geom::PathVector pv(Geom::Path(Geom::Rect(Geom::Point(-10, 18), Geom::Point(100, 90))));
    SpamRgn rgn0(pv*Geom::Scale(0.5, 0.5));

    cv::Point2d cg = SpamRgn(pv).Centroid();
    SpamRgn rgn1(pv * Geom::Translate(-cg.x, -cg.y)*Geom::Scale(0.5, 0.5)*Geom::Translate(cg.x*0.5, cg.y*0.5));

    EXPECT_DOUBLE_EQ(rgn0.Centroid().x, rgn1.Centroid().x);
    EXPECT_DOUBLE_EQ(rgn0.Centroid().y, rgn1.Centroid().y);
}

TEST(RgnPyramidTest, Polygon)
{
    Geom::PathVector pv;
    Geom::PathBuilder pb(pv);
    pb.moveTo(Geom::Point(10, 10));
    pb.lineTo(Geom::Point(5, 20));
    pb.lineTo(Geom::Point(30, 50));
    pb.lineTo(Geom::Point(60, 20));
    pb.lineTo(Geom::Point(50, 5));
    pb.closePath();

    SpamRgn rgn0(pv*Geom::Scale(0.5, 0.5));

    cv::Point2d cg = SpamRgn(pv).Centroid();
    SpamRgn rgn1(pv * Geom::Translate(-cg.x, -cg.y)*Geom::Scale(0.5, 0.5)*Geom::Translate(cg.x*0.5, cg.y*0.5));

    EXPECT_DOUBLE_EQ(rgn0.Centroid().x, rgn1.Centroid().x);
    EXPECT_DOUBLE_EQ(rgn0.Centroid().y, rgn1.Centroid().y);
}

TEST(RgnPyramidTest, Disjoint)
{
    Geom::PathVector pv;
    Geom::PathBuilder pb(pv);
    pb.append(Geom::Path(Geom::Circle(Geom::Point(100, 200), 30)));
    pb.closePath();
    pb.append(Geom::Path(Geom::Rect(Geom::Point(-10, 18), Geom::Point(30, 30))));
    pb.closePath();

    SpamRgn rgn0(pv*Geom::Scale(0.5, 0.5));

    cv::Point2d cg = SpamRgn(pv).Centroid();
    SpamRgn rgn1(pv * Geom::Translate(-cg.x, -cg.y)*Geom::Scale(0.5, 0.5)*Geom::Translate(cg.x*0.5, cg.y*0.5));

    EXPECT_DOUBLE_EQ(rgn0.Centroid().x, rgn1.Centroid().x);
    EXPECT_DOUBLE_EQ(rgn0.Centroid().y, rgn1.Centroid().y);
}

}