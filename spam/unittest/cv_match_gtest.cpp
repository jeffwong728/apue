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

TEST(CVImageRotateTest, Dot)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Point center{ 50, 30 };
    img.at<uint8_t>(center) = 0xFF;
    UnitTestHelper::WriteImage(img, "rotate_dot_src.png");

    cv::Mat rotMat = cv::getRotationMatrix2D(center, 30, 1.0);
    cv::Mat dst;
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));
    UnitTestHelper::WriteImage(dst, "rotate_dot_dst.png");
}

TEST(CVImageRotateTest, Cross)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Point center{50, 30};
    cv::drawMarker(img, center, CV_RGB(255, 255, 255), cv::MARKER_CROSS, 10, 1);
    UnitTestHelper::WriteImage(img, "rotate_cross_src.png");

    cv::Mat rotMat = cv::getRotationMatrix2D(center, 30, 1.0);
    cv::Mat dst;
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));
    UnitTestHelper::WriteImage(dst, "rotate_cross_dst.png");
}

TEST(CVImageRotateTest, Rectangle)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Rect rect{ 30, 5, 30, 10 };
    cv::rectangle(img, rect, CV_RGB(255, 255, 255));
    UnitTestHelper::WriteImage(img, "rotate_rect_src.png");

    cv::Mat dst;
    cv::Mat rotMat = cv::getRotationMatrix2D((rect.tl() + rect.br())/2, 60, 1.0);
    cv::warpAffine(img, dst, rotMat, cv::Size(img.cols, img.rows));

    dst += img;
    UnitTestHelper::WriteImage(dst, "rotate_rect_dst.png");
}

TEST(CVImageTranslateTest, Rectangle)
{
    cv::Mat img(48, 64, CV_8UC1, cv::Scalar());
    cv::Rect rect{ 30, 5, 30, 10 };
    cv::rectangle(img, rect, CV_RGB(255, 255, 255));
    UnitTestHelper::WriteImage(img, "translate_rect_src.png");

    cv::Mat transImg;
    cv::Mat transMat = (cv::Mat_<double>(2, 3) << 1, 0, -13, 0, 1, 14);
    cv::warpAffine(img, transImg, transMat, cv::Size(img.cols, img.rows));

    cv::Mat dst;
    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point(32, 24), 60, 1.0);
    cv::warpAffine(transImg, dst, rotMat, cv::Size(transImg.cols, transImg.rows));

    dst += transImg;
    UnitTestHelper::WriteImage(dst, "translate_rect_dst.png");
}

TEST(CVMatTest, Create)
{
    cv::Mat topImg;
    topImg.create(48, 64, CV_16SC1);
    topImg.create(48, 64, CV_16SC1);
    topImg = cv::Scalar(0);

    cv::Mat angImg;
    angImg.create(48, 64, CV_32F);
    angImg = cv::Scalar(0);

    EXPECT_EQ(topImg.rows, 48);
    EXPECT_EQ(topImg.cols, 64);
    EXPECT_EQ(topImg.elemSize1(), 2);
    EXPECT_EQ(topImg.step1(), 64);
    EXPECT_EQ(angImg.rows, 48);
    EXPECT_EQ(angImg.cols, 64);
    EXPECT_EQ(angImg.elemSize1(), 4);
    EXPECT_EQ(angImg.step1(), 64);
}

}