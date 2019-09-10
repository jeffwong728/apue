#define BOOST_TEST_MODULE TestShapeTmpl
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <vectorclass/instrset.h>
#include <vectorclass/vectorclass.h>
#include <opencv2/highgui.hpp>
#include <ui/proc/rgn.h>
#include <ui/proc/basic.h>
#include <ui/proc/shapetmpl.h>
#ifdef free
#undef free
#endif
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

struct TestShapeTmplConfig
{
    TestShapeTmplConfig()
    {
        std::cout << "Global setup" << std::endl; 
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
        std::cout << "Vcl detected instruction set: "<< vcl::instrset_detect() << std::endl;
    }

    ~TestShapeTmplConfig()
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_GLOBAL_FIXTURE(TestShapeTmplConfig);


BOOST_AUTO_TEST_CASE(test_ShapeTmpl_Create_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2200, 2050));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    ShapeTmplCreateData tmplCreateData{ {grayImg , tmplRgn, roi, -1, 2, 5}, 20, 30 };

    tbb::tick_count t1 = tbb::tick_count::now();
    ShapeTemplate tmpl;
    SpamResult sr = tmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create Shape template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerShapeData> &tmplDatas = tmpl.GetTmplDatas();
    for (int l=0; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerShapeData &lsd = tmplDatas[l];
        const auto &tmplDatas = lsd.tmplDatas;
        for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
        {
            int i = 0;
            const ShapeTemplData &std = tmplDatas[t];
            const int width = std.maxPoint.x - std.minPoint.x + 1;
            const int height = std.maxPoint.y - std.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(0, 0, 0));
            for (const cv::Point &pt : std.edgeLocs)
            {
                tmplMat.at<uint8_t>(pt - std.minPoint) = 0xFF;
            }

            std::string fileName = std::string("mista_tmpl_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
            UnitTestHelper::WriteImage(tmplMat, fileName);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_ShapeTmpl_Mista, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2300, 2150));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    const Geom::PathVector roi;
    const ShapeTmplCreateData tmplCreateData{ {grayImg , tmplRgn, roi, -180, 360, 6}, 20, 30 };

    tbb::tick_count t1 = tbb::tick_count::now();
    ShapeTemplate tmpl;
    SpamResult sr = tmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create big shape template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0, score = 0;

    t1 = tbb::tick_count::now();
    sr = tmpl.matchShapeTemplate(grayImg, 0.9f, 20, 0.9f, pos, angle, score);
    t2 = tbb::tick_count::now();
    cv::Point2f pt = tmpl.GetCenter();
    BOOST_TEST_MESSAGE("Match big shape (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::string fileName = std::string("big_shape_top_layer_score.png");
    UnitTestHelper::WriteImage(tmpl.GetTopScoreMat(), fileName);

    if (SpamResult::kSR_SUCCESS == sr)
    {
        UnitTestHelper::Color color{ 255, 0, 0, 255 };
        const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
        UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
        tmpl.DrawTemplate(colorImg, pos, angle);
        UnitTestHelper::WriteImage(colorImg, std::string("mista_shape_match.png"));
        BOOST_TEST_MESSAGE("Shape (mista.png) matched at: (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);
    }
}

BOOST_AUTO_TEST_CASE(test_ShapeTmpl_NCC_Board, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(185, 198), Geom::Point(399, 286));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    const ShapeTmplCreateData tmplCreateData{ {grayImg , tmplRgn, roi, -180, 359, 4}, 8, 10 };

    tbb::tick_count t1 = tbb::tick_count::now();
    ShapeTemplate tmpl;
    SpamResult sr = tmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create shape template (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_REQUIRE_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    boost::filesystem::path utRootDir = std::getenv("SPAM_UNITTEST_ROOT");
    utRootDir.append("idata");
    utRootDir.append("images");
    utRootDir.append("board");

    boost::filesystem::path baseDir = std::getenv("SPAM_UNITTEST_ROOT");
    baseDir.append("idata");
    for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(utRootDir))
    {
        if (boost::filesystem::is_regular_file(x.path()) && x.path().extension() == ".png")
        {
            cv::Point2f pos;
            float angle = 0, score = 0;
            std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(boost::filesystem::relative(x.path(), baseDir).string());

            t1 = tbb::tick_count::now();
            sr = tmpl.matchShapeTemplate(grayImg, 0.8f, 8, 0.9f, pos, angle, score);
            t2 = tbb::tick_count::now();
            BOOST_TEST_MESSAGE(std::string("Match shape template (") + x.path().filename().string() + ") " \
                << (t2 - t1).seconds() * 1000 << "ms (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);

            std::string fileName = std::string("top_layer_") + x.path().filename().string();
            UnitTestHelper::WriteImage(tmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = tmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                tmpl.DrawTemplate(colorImg, pos, angle);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(test_ShapeTmpl_NCC_Cap, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\cap_illumination\\cap_illumination_01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(370, 495), Geom::Point(875, 675));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    const ShapeTmplCreateData tmplCreateData{ {grayImg , tmplRgn, roi, -180, 359, 5}, 15, 30 };

    tbb::tick_count t1 = tbb::tick_count::now();
    ShapeTemplate tmpl;
    SpamResult sr = tmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create shape template (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_REQUIRE_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    boost::filesystem::path utRootDir = std::getenv("SPAM_UNITTEST_ROOT");
    utRootDir.append("idata");
    utRootDir.append("images");
    utRootDir.append("cap_illumination");

    boost::filesystem::path baseDir = std::getenv("SPAM_UNITTEST_ROOT");
    baseDir.append("idata");
    for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(utRootDir))
    {
        if (boost::filesystem::is_regular_file(x.path()) && x.path().extension() == ".png")
        {
            cv::Point2f pos;
            float angle = 0, score = 0;
            std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(boost::filesystem::relative(x.path(), baseDir).string());

            t1 = tbb::tick_count::now();
            sr = tmpl.matchShapeTemplate(grayImg, 0.6f, 10, 0.9f, pos, angle, score);
            t2 = tbb::tick_count::now();
            BOOST_TEST_MESSAGE(std::string("Match shape template (") + x.path().filename().string() + ") " \
                << (t2 - t1).seconds() * 1000 << "ms (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);

            std::string fileName = std::string("top_layer_") + x.path().filename().string();
            UnitTestHelper::WriteImage(tmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = tmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                tmpl.DrawTemplate(colorImg, pos, angle);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(test_ShapeTmpl_NCC_Pendulum, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\pendulum\\pendulum_07.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(201, 58), Geom::Point(221, 117));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    const ShapeTmplCreateData tmplCreateData{ {grayImg , tmplRgn, roi, -180, 359, 3}, 10, 15 };

    tbb::tick_count t1 = tbb::tick_count::now();
    ShapeTemplate tmpl;
    SpamResult sr = tmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create shape template (pendulum_07.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_REQUIRE_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    boost::filesystem::path utRootDir = std::getenv("SPAM_UNITTEST_ROOT");
    utRootDir.append("idata");
    utRootDir.append("images");
    utRootDir.append("pendulum");

    boost::filesystem::path baseDir = std::getenv("SPAM_UNITTEST_ROOT");
    baseDir.append("idata");
    for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(utRootDir))
    {
        if (boost::filesystem::is_regular_file(x.path()) && x.path().extension() == ".png")
        {
            cv::Point2f pos;
            float angle = 0, score = 0;
            std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(boost::filesystem::relative(x.path(), baseDir).string());

            t1 = tbb::tick_count::now();
            sr = tmpl.matchShapeTemplate(grayImg, 0.8f, 8, 0.5f, pos, angle, score);
            t2 = tbb::tick_count::now();
            BOOST_TEST_MESSAGE(std::string("Match shape template (") + x.path().filename().string() + ") " \
                << (t2 - t1).seconds() * 1000 << "ms (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);

            std::string fileName = std::string("top_layer_") + x.path().filename().string();
            UnitTestHelper::WriteImage(tmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = tmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                tmpl.DrawTemplate(colorImg, pos, angle);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}