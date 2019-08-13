#define BOOST_TEST_MODULE TestPixelTmpl
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <vectorclass/instrset.h>
#include <vectorclass/vectorclass.h>
#include <opencv2/highgui.hpp>
#include <ui/proc/rgn.h>
#include <ui/proc/basic.h>
#include <ui/proc/pixeltmpl.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

struct TestPixelTmplConfig
{
    TestPixelTmplConfig()
    {
        std::cout << "Global setup" << std::endl; 
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
        std::cout << "Vcl detected instruction set: "<< vcl::instrset_detect() << std::endl;
    }

    ~TestPixelTmplConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_GLOBAL_FIXTURE(TestPixelTmplConfig);

BOOST_AUTO_TEST_CASE(test_PixelTmpl_Create_Small, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(143, 121), Geom::Point(465, 177));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    tmplRgn.push_back(Geom::Path(Geom::Rect(Geom::Point(140, 311), Geom::Point(463, 368))));
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -60, 120, 4, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreatePixelTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerTmplData> &tmplDatas = pixelTmpl.GetTmplDatas();
    for (int l=0; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerTmplData &ltd = tmplDatas[l];
        for (int t = 0; t < static_cast<int>(ltd.tmplDatas.size()); ++t)
        {
            int i = 0;
            const PixelTmplData &ptd = ltd.tmplDatas[t];
            const int width = ptd.maxPoint.x - ptd.minPoint.x + 1;
            const int height = ptd.maxPoint.y - ptd.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(128, 128, 128));
            for (const cv::Point &pt : ptd.pixlLocs)
            {
                tmplMat.at<uint8_t>(pt - ptd.minPoint) = static_cast<uint8_t>(boost::get<std::vector<int16_t>>(ptd.pixlVals)[i++]);
            }

            std::string fileName = std::string("mista_tmpl_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
            UnitTestHelper::WriteImage(tmplMat, fileName);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_Create_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2200, 2050));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -60, 120, 5, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreatePixelTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerTmplData> &tmplDatas = pixelTmpl.GetTmplDatas();
    for (int l = 3; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerTmplData &ltd = tmplDatas[l];
        for (int t = 0; t < static_cast<int>(ltd.tmplDatas.size()); ++t)
        {
            int i = 0;
            const PixelTmplData &ptd = ltd.tmplDatas[t];
            const int width = ptd.maxPoint.x - ptd.minPoint.x + 1;
            const int height = ptd.maxPoint.y - ptd.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(128, 128, 128));
            for (const cv::Point &pt : ptd.pixlLocs)
            {
                tmplMat.at<uint8_t>(pt - ptd.minPoint) = static_cast<uint8_t>(boost::get<std::vector<int16_t>>(ptd.pixlVals)[i++]);
            }

            std::string fileName = std::string("mista_tmpl_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
            UnitTestHelper::WriteImage(tmplMat, fileName);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_SAD_Small, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(143, 121), Geom::Point(465, 177));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    tmplRgn.push_back(Geom::Path(Geom::Rect(Geom::Point(140, 311), Geom::Point(463, 368))));
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -180, 359, 4, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreatePixelTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0;
    t1 = tbb::tick_count::now();
    pixelTmpl.matchTemplate(grayImg, 10, pos, angle);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Match pixel template (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");

    const cv::Mat &topScoreMat = pixelTmpl.TopScoreMat();
    cv::Mat dst;
    cv::equalizeHist(topScoreMat, dst);

    std::string fileName = std::string("board_01_top_layer_score.png");
    UnitTestHelper::WriteImage(dst, fileName);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_SAD_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2300, 2150));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -60, 120, 6, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreatePixelTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0;
    t1 = tbb::tick_count::now();
    pixelTmpl.matchTemplate(grayImg, 10, pos, angle);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Match pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    const cv::Mat &topScoreMat = pixelTmpl.TopScoreMat();
    cv::Mat dst;
    cv::equalizeHist(topScoreMat, dst);

    std::string fileName = std::string("mista_tmpl_top_layer_score.png");
    UnitTestHelper::WriteImage(dst, fileName);
}