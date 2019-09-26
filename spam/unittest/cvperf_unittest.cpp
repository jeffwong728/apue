#define BOOST_TEST_MODULE TestCVPerf
#include "helper.h"
#include <ui/proc/gradient.h>
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>

struct TestCVPerfConfig
{
    TestCVPerfConfig()
    {
        std::cout << "Global setup" << std::endl;
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
    }

    ~TestCVPerfConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_AUTO_TEST_CASE(test_CV_threshold_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(numLabels, 942);
    BOOST_TEST_MESSAGE("CV connected components spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;

    t1 = tbb::tick_count::now();
    cv::findContours(binImg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV find contours spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    t1 = tbb::tick_count::now();
    cv::inRange(grayImg, 155, 255, binImg);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV in range spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
}

BOOST_AUTO_TEST_CASE(test_CV_buildPyramid_Performance_0, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    std::vector<cv::Mat> pyrs;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::buildPyramid(grayImg, pyrs, 4, cv::BORDER_REFLECT);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_CHECK_EQUAL(pyrs.size(), 5);
    BOOST_TEST_MESSAGE("CV build pyramid spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    cv::Mat borderImg;
    t1 = tbb::tick_count::now();
    cv::copyMakeBorder(pyrs[4], borderImg, 64, 64, 64, 64, cv::BORDER_CONSTANT, 0);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV copy make border spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    int id = 0;
    for (const cv::Mat &pyr : pyrs)
    {
        std::string fileName = std::string("mista_pyr_") + std::to_string(id++) + ".png";
        UnitTestHelper::WriteImage(pyr, fileName);
    }

    UnitTestHelper::WriteImage(borderImg, std::string("mista_pyr_border.png"));
}

BOOST_AUTO_TEST_CASE(test_CV_buildPyramid_Performance_1, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    std::vector<cv::Mat> pyrs;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::buildPyramid(grayImg, pyrs, 5, cv::BORDER_REFLECT);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_CHECK_EQUAL(pyrs.size(), 6);
    BOOST_TEST_MESSAGE("CV build pyramid spend (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");

    cv::Mat borderImg;
    t1 = tbb::tick_count::now();
    cv::copyMakeBorder(pyrs[4], borderImg, 64, 64, 64, 64, cv::BORDER_CONSTANT, 0);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV copy make border spend (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");

    int id = 0;
    for (const cv::Mat &pyr : pyrs)
    {
        std::string fileName = std::string("board-01_pyr_") + std::to_string(id++) + ".png";
        UnitTestHelper::WriteImage(pyr, fileName);
    }

    UnitTestHelper::WriteImage(borderImg, std::string("board-01_pyr_border.png"));
}

BOOST_AUTO_TEST_CASE(test_CV_Sobel_Performance_0, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    std::vector<cv::Mat> pyrs;
    
    cv::buildPyramid(grayImg, pyrs, 4, cv::BORDER_REFLECT);
    BOOST_CHECK_EQUAL(pyrs.size(), 5);

    int id = 0;
    for (const cv::Mat &pyr : pyrs)
    {
        cv::Mat dx, dy, sdx, sdy, xDiff, yDiff, ndx, ndy;
        tbb::tick_count t1 = tbb::tick_count::now();
        SpamGradient::SobelNormalize(pyr, ndx, ndy, cv::Rect(0, 0, pyr.cols, pyr.rows), 30);
        tbb::tick_count t2 = tbb::tick_count::now();
        BOOST_TEST_MESSAGE("CV spatial gradient level " << id++ << " spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

        cv::spatialGradient(pyr, dx, dy);
        SpamGradient::Sobel(pyr, sdx, sdy, cv::Rect(0, 0, pyr.cols, pyr.rows));
        cv::Mat reducedDx{ dx, cv::Range(1, pyr.rows-1), cv::Range(1, pyr.cols - 1) };
        cv::Mat reducedSDx{ sdx, cv::Range(1, pyr.rows - 1), cv::Range(1, pyr.cols - 1) };
        cv::Mat reducedDy{ dy, cv::Range(1, pyr.rows - 1), cv::Range(1, pyr.cols - 1) };
        cv::Mat reducedSDy{ sdy, cv::Range(1, pyr.rows - 1), cv::Range(1, pyr.cols - 1) };
        cv::absdiff(reducedDx, reducedSDx, xDiff);
        cv::absdiff(reducedDy, reducedSDy, yDiff);

        double minValX=-1, minValY=-1, maxValX=-1, maxValY=-1;
        cv::minMaxIdx(xDiff, &minValX, &maxValX);
        cv::minMaxIdx(yDiff, &minValY, &maxValY);
        BOOST_CHECK_SMALL(maxValX, 1e-12);
        BOOST_CHECK_SMALL(maxValY, 1e-12);

        cv::Mat fdx, fdy;
        reducedSDx.convertTo(fdx, CV_32FC1);
        reducedSDy.convertTo(fdy, CV_32FC1);

        cv::Mat magSobel;
        cv::magnitude(fdx, fdy, magSobel);
        cv::divide(fdx, magSobel, fdx);
        cv::divide(fdy, magSobel, fdy);
        cv::patchNaNs(fdx);
        cv::patchNaNs(fdy);

        cv::Mat reducedNdx{ ndx, cv::Range(1, pyr.rows - 1), cv::Range(1, pyr.cols - 1) };
        cv::Mat reducedNdy{ ndy, cv::Range(1, pyr.rows - 1), cv::Range(1, pyr.cols - 1) };

        minValX = -1, minValY = -1, maxValX = -1, maxValY = -1;
        cv::absdiff(fdx, reducedNdx, xDiff);
        cv::absdiff(fdy, reducedNdy, yDiff);
        cv::minMaxIdx(xDiff, &minValX, &maxValX);
        cv::minMaxIdx(yDiff, &minValY, &maxValY);
        BOOST_CHECK_SMALL(maxValX, 1e-3);
        BOOST_CHECK_SMALL(maxValY, 1e-3);

        //xDst.convertTo(xDst, CV_8UC1);
        //yDst.convertTo(yDst, CV_8UC1);
        //cv::convertScaleAbs(sdx, fdx, 255);
    }
}

BOOST_AUTO_TEST_CASE(test_CV_Edge_Cany, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\pendulum\\pendulum_07.png");

    std::vector<cv::Mat> pyrs;

    cv::buildPyramid(grayImg, pyrs, 3, cv::BORDER_REFLECT);
    BOOST_CHECK_EQUAL(pyrs.size(), 4);

    int id = 0;
    for (const cv::Mat &pyr : pyrs)
    {
        cv::Mat dx, dy, edges;
        SpamGradient::Sobel(pyr, dx, dy, cv::Rect(0, 0, pyr.cols, pyr.rows));

        //dx *= 1.0 / 8;
        //dy *= 1.0 / 8;
        cv::Canny(dx, dy, edges, 10, 15, true);

        std::string fileName = std::string("mista_pyr_edge_") + std::to_string(id++) + ".png";
        UnitTestHelper::WriteImage(edges, fileName);
    }
}

BOOST_AUTO_TEST_CASE(test_CV_GaussianBlur_Performance_0, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::GaussianBlur(grayImg, dst, cv::Size(5, 5), cv::BORDER_CONSTANT);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV gaussian blur spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_GaussianBlur.png");
}

BOOST_AUTO_TEST_CASE(test_CV_medianBlur_Performance_0, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Mat dst;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::medianBlur(grayImg, dst, 5);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV median blur spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_medianBlur.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_ERODE_RECT_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex erode rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_erode_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_DILATE_RECT_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex dilate rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_dilate_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_OPEN_RECT_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex open rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_open_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_CLOSE_RECT_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex close rect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_close_rect.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_ERODE_ELLIPSE_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex erode ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_erode_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_DILATE_ELLIPSE_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex dilate ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_dilate_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_OPEN_ELLIPSE_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex open ELLIPSE spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_open_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_morphologyEx_CLOSE_ELLIPSE_Performance_0, *boost::unit_test::enable_if<false>())
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
    BOOST_CHECK_EQUAL(grayImg.rows, dst.rows);
    BOOST_TEST_MESSAGE("CV morphologyex close ellipse spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    UnitTestHelper::WriteImage(dst, "mista_morphologyex_close_ellipse.png");
}

BOOST_AUTO_TEST_CASE(test_CV_ORB, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImgTemp, colorImgTemp;
    std::tie(grayImgTemp, colorImgTemp) = UnitTestHelper::GetGrayScaleImage("box.png");

    cv::Mat grayImgTarget, colorImgTarget;
    std::tie(grayImgTarget, colorImgTarget) = UnitTestHelper::GetGrayScaleImage("box_in_scene.png");

    cv::Ptr<cv::ORB> detector = cv::ORB::create();
    cv::Ptr<cv::flann::IndexParams> indexParams = cv::makePtr<cv::flann::LshIndexParams>(6, 12, 1); // instantiate LSH index parameters
    cv::Ptr<cv::flann::SearchParams> searchParams = cv::makePtr<cv::flann::SearchParams>(50);       // instantiate flann search parameters
    cv::Ptr<cv::FlannBasedMatcher> matcher = cv::makePtr<cv::FlannBasedMatcher>(indexParams, searchParams);

    cv::Mat descriptors1, descriptors2;
    std::vector<cv::KeyPoint> keyPoints1, keyPoints2;
    detector->detectAndCompute(grayImgTemp, cv::Mat(), keyPoints1, descriptors1);
    detector->detectAndCompute(grayImgTarget, cv::Mat(), keyPoints2, descriptors2);

    std::vector<cv::DMatch> goodMatches;
    std::vector<std::vector<cv::DMatch>> matches;;
    matcher->knnMatch(descriptors1, descriptors2, matches, 2);

    constexpr double  RATIO = 0.75;
    for (unsigned int i = 0; i < matches.size(); ++i) 
    {
        if (matches[i][0].distance < matches[i][1].distance * RATIO)
        {
            goodMatches.push_back(matches[i][0]);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_Solve)
{
    int i = 0;
    cv::Mat A(27, 10, CV_64FC1);
    for (int r = -1; r < 2; ++r)
    {
        for (int c = -1; c < 2; ++c)
        {
            for (int a = -1; a < 2; ++a)
            {
                double *R = A.ptr<double>(i++);
                R[0] = 1;
                R[1] = r; R[2] = c; R[3] = a;
                R[4] = r * c; R[5] = r * a; R[6] = c * a;
                R[7] = r * r; R[8] = c * c; R[9] = a * a;
            }
        }
    }

    cv::Mat TA, IA;
    cv::transpose(A, TA);
    cv::invert(TA * A, IA);
    cv::Mat B = IA * TA;
    BOOST_CHECK_EQUAL(B.rows, 10);
    BOOST_CHECK_EQUAL(B.cols, 27);
}