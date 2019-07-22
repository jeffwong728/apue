#define BOOST_TEST_MODULE TestCVPerf
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <opencv2/highgui.hpp>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>

BOOST_AUTO_TEST_CASE(test_CV_threshold_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat labels;
    cv::Mat binImg;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV threshold spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    t1 = tbb::tick_count::now();
    int numLabels = cv::connectedComponents(binImg, labels, 8, CV_32S, cv::CCL_GRANA);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV connected components spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;

    t1 = tbb::tick_count::now();
    cv::findContours(binImg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV find contours spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
}

BOOST_AUTO_TEST_CASE(test_CV_buildPyramid_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    std::vector<cv::Mat> pyrs;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::buildPyramid(grayImg, pyrs, 4, cv::BORDER_REFLECT);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV build pyramid spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    int id = 0;
    for (const cv::Mat &pyr : pyrs)
    {
        std::string fileName = std::string("mista_pyr_") + std::to_string(id++) + ".png";
        UnitTestHelper::WriteImage(pyr, fileName);
    }
}

BOOST_AUTO_TEST_CASE(test_CV_GaussianBlur_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::GaussianBlur(grayImg, dst, cv::Size(11, 11), cv::BORDER_CONSTANT);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV gaussian blur spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_GaussianBlur.png");
}

BOOST_AUTO_TEST_CASE(test_CV_medianBlur_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::medianBlur(grayImg, dst, 5);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV median blur spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_medianBlur.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_ERODE_RECT_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
    cv::morphologyEx(binImg, dst, cv::MORPH_ERODE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex erode rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_erode_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_DILATE_RECT_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
    cv::morphologyEx(binImg, dst, cv::MORPH_DILATE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex dilate rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_dilate_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_OPEN_RECT_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
    cv::morphologyEx(binImg, dst, cv::MORPH_OPEN, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex open rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_open_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_CLOSE_RECT_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
    cv::morphologyEx(binImg, dst, cv::MORPH_CLOSE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex close rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_close_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_ERODE_ELLIPSE_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binImg, dst, cv::MORPH_ERODE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex erode ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_erode_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_DILATE_ELLIPSE_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binImg, dst, cv::MORPH_DILATE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex dilate ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_dilate_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_OPEN_ELLIPSE_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binImg, dst, cv::MORPH_OPEN, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex open ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_open_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_CLOSE_ELLIPSE_Performance_0)
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat binImg;
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binImg, dst, cv::MORPH_CLOSE, kernel);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV morphologyex close ellipse spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_close_ellipse.png");
}