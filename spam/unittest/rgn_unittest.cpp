#define BOOST_TEST_MODULE TestSpamRgn
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <ui/proc/rgn.h>
#include <ui/proc/basic.h>
#include <opencv2/highgui.hpp>
#include <opencv2/mvlab.hpp>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

struct TestSpamRgnConfig 
{
    TestSpamRgnConfig() 
    {
        std::cout << "Global setup" << std::endl; 
        std::cout << "Default number of threads: " << tbb::task_scheduler_init::default_num_threads() << std::endl;
        BasicImgProc::Initialize(tbb::task_scheduler_init::default_num_threads());
    }

    ~TestSpamRgnConfig() 
    {
        std::cout << "Clear test images cache..." << std::endl;
        UnitTestHelper::ClearImagesCache();
    }
};

BOOST_GLOBAL_FIXTURE(TestSpamRgnConfig);

BOOST_AUTO_TEST_CASE(test_SpamRgn_0_Area)
{
    SpamRgn rgn;
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 0.0, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().x, 0.0, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().y, 0.0, 0.1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_1_Area)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 1.0, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().x, 0.0, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().y, 0.0, 0.1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Area)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 1);
    rgn.AddRun(0, 3, 10);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 8.0, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().x, 42.0 / 8, 0.1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Centroid().y, 0.0, 0.1);

    rgn.clear();
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Connect_Rgn_2, *boost::unit_test::enable_if<true>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 0, 2);
    rgn.AddRun(1, 7, 10);
    rgn.AddRun(2, 2, 7);
    rgn.AddRun(4, 1, 5);
    rgn.AddRun(4, 7, 10);
    rgn.AddRun(4, 12, 18);
    rgn.AddRun(5, 1, 20);

    SPSpamRgnVector rgns = rgn.Connect();
    BOOST_REQUIRE_EQUAL(rgns->size(), 2);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Connect_Rgn_3, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);
    BOOST_CHECK_EQUAL(rgn->GetData().size(), 234794);

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

    t1 = tbb::tick_count::now();
    SPSpamRgnVector lrgns = BasicImgProc::Connect(labels, numLabels - 1);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam label connect spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    cv::Mat stats;
    cv::Mat centroids;
    t1 = tbb::tick_count::now();
    cv::connectedComponentsWithStats(binImg, labels, stats, centroids, 8, CV_32S, cv::CCL_GRANA);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV connected components with stats spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;

    t1 = tbb::tick_count::now();
    cv::findContours(binImg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV find contours spend (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    int numExtContours = 0;
    for (int i = 0; i < contours.size(); i = hierarchy[i][0])
    {
        numExtContours += 1;
    }

    SPSpamRgnVector rgns = rgn->Connect();
    BOOST_REQUIRE_EQUAL(rgns->size(), numLabels - 1);
    BOOST_REQUIRE_EQUAL(rgns->size(), numExtContours);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Connect_Rgn_4, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("digits.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 151, 255);
    BOOST_CHECK_EQUAL(rgn->GetData().size(), 89084);

    cv::Mat labels;
    cv::Mat binImg;
    tbb::tick_count t1 = tbb::tick_count::now();
    cv::threshold(grayImg, binImg, 150, 255, cv::THRESH_BINARY);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV threshold spend (digits.png): " << (t2 - t1).seconds() * 1000 << "ms");

    t1 = tbb::tick_count::now();
    int numLabels = cv::connectedComponents(binImg, labels, 8, CV_32S, cv::CCL_GRANA);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV connected components spend (digits.png): " << (t2 - t1).seconds() * 1000 << "ms");

    t1 = tbb::tick_count::now();
    SPSpamRgnVector rgns = BasicImgProc::Connect(labels, numLabels);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam label connect spend (digits.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;

    t1 = tbb::tick_count::now();
    cv::findContours(binImg, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("CV find contours spend (digits.png): " << (t2 - t1).seconds() * 1000 << "ms");

    int numExtContours = 0;
    for (int i = 0; i < contours.size(); i = hierarchy[i][0])
    {
        numExtContours += 1;
    }

    t1 = tbb::tick_count::now();
    rgns = rgn->Connect();
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam connect spend (digits.png): " << (t2 - t1).seconds() * 1000 << "ms");

    BOOST_REQUIRE_EQUAL(rgns->size(), numLabels - 1);
    BOOST_REQUIRE_EQUAL(rgns->size(), numExtContours);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_0, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 0);
    BOOST_CHECK_EQUAL(re_list.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_1, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 1);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 1);
    BOOST_CHECK_EQUAL(re_list.size(), 3);
    BOOST_CHECK_EQUAL(re_list[0].X, 0);
    BOOST_CHECK_EQUAL(re_list[0].Y, 0);
    BOOST_CHECK_EQUAL(re_list[0].CODE, 0);
    BOOST_CHECK_EQUAL(re_list[0].LINK, 0);
    BOOST_CHECK_EQUAL(re_list[0].W_LINK, 1);
    BOOST_CHECK_EQUAL(re_list[0].QI, 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_2, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(0, 1, 2);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 1);
    BOOST_CHECK_EQUAL(re_list.size(), 3);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_3, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(0, 5, 10);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 1);
    BOOST_CHECK_EQUAL(re_list.size(), 3);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_4, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 5, 10);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 1);
    BOOST_CHECK_EQUAL(re_list.size(), 3);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_5, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 3);
    rgn.AddRun(1, 5, 6);
    rgn.AddRun(2, 2, 6);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 3);
    BOOST_CHECK_EQUAL(re_list.size(), 7);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[3].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[4].CODE, 10);
    BOOST_CHECK_EQUAL(re_list[5].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[6].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_6, *boost::unit_test::enable_if<true>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 3);
    rgn.AddRun(1, 5, 6);
    rgn.AddRun(2, 1, 7);
    rgn.AddRun(3, 1, 3);
    rgn.AddRun(3, 5, 7);
    rgn.AddRun(4, 2, 3);
    rgn.AddRun(4, 5, 6);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 7);
    BOOST_CHECK_EQUAL(re_list.size(), 15);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[3].CODE, 3);
    BOOST_CHECK_EQUAL(re_list[4].CODE, 10);
    BOOST_CHECK_EQUAL(re_list[5].CODE, 8);
    BOOST_CHECK_EQUAL(re_list[6].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[7].CODE, 9);
    BOOST_CHECK_EQUAL(re_list[8].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[9].CODE, 4);
    BOOST_CHECK_EQUAL(re_list[10].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[11].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[12].CODE, 7);
    BOOST_CHECK_EQUAL(re_list[13].CODE, 5);
    BOOST_CHECK_EQUAL(re_list[14].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_7, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 5);
    rgn.AddRun(2, 2, 5);
    rgn.AddRun(3, 2, 5);
    rgn.AddRun(4, 2, 5);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 4);
    BOOST_CHECK_EQUAL(re_list.size(), 9);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[3].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[4].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[5].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[6].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[7].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[8].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_8, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(2, 2, 8);
    rgn.AddRun(3, 3, 5);
    rgn.AddRun(3, 7, 9);
    rgn.AddRun(4, 3, 4);
    rgn.AddRun(4, 8, 9);
    rgn.AddRun(5, 3, 6);
    rgn.AddRun(5, 8, 9);
    rgn.AddRun(6, 2, 8);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 8);
    BOOST_CHECK_EQUAL(re_list.size(), 17);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 4);
    BOOST_CHECK_EQUAL(re_list[3].CODE, 9);
    BOOST_CHECK_EQUAL(re_list[4].CODE, 8);
    BOOST_CHECK_EQUAL(re_list[5].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[6].CODE, 7);
    BOOST_CHECK_EQUAL(re_list[7].CODE, 4);
    BOOST_CHECK_EQUAL(re_list[8].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[9].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[10].CODE, 8);
    BOOST_CHECK_EQUAL(re_list[11].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[12].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[13].CODE, 3);
    BOOST_CHECK_EQUAL(re_list[14].CODE, 10);
    BOOST_CHECK_EQUAL(re_list[15].CODE, 7);
    BOOST_CHECK_EQUAL(re_list[16].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_9, *boost::unit_test::enable_if<true>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 8);
    rgn.AddRun(2, 2, 8);
    rgn.AddRun(9, 1, 10);
    rgn.AddRun(10, 1, 10);

    RunTypeDirectionEncoder encoder(rgn);
    RD_LIST re_list = encoder.encode();
    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 4);
    BOOST_CHECK_EQUAL(re_list.size(), 9);
    BOOST_CHECK_EQUAL(re_list[1].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[2].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[3].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[4].CODE, 5);
    BOOST_CHECK_EQUAL(re_list[5].CODE, 1);
    BOOST_CHECK_EQUAL(re_list[6].CODE, 2);
    BOOST_CHECK_EQUAL(re_list[7].CODE, 6);
    BOOST_CHECK_EQUAL(re_list[8].CODE, 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_0, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(0, 1, 2);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_CHECK_EQUAL(outers.size(), 1);
    BOOST_CHECK_EQUAL(outer.size(), 4);
    BOOST_CHECK_EQUAL(holes.size(), 0);
    BOOST_CHECK_EQUAL(outer[0].x, 1);
    BOOST_CHECK_EQUAL(outer[0].y, 0);
    BOOST_CHECK_EQUAL(outer[1].x, 1);
    BOOST_CHECK_EQUAL(outer[1].y, 1);
    BOOST_CHECK_EQUAL(outer[2].x, 2);
    BOOST_CHECK_EQUAL(outer[2].y, 1);
    BOOST_CHECK_EQUAL(outer[3].x, 2);
    BOOST_CHECK_EQUAL(outer[3].y, 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_1, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 1, 5);
    rgn.AddRun(2, 1, 5);
    rgn.AddRun(3, 1, 5);
    rgn.AddRun(4, 1, 5);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_CHECK_EQUAL(outers.size(), 1);
    BOOST_CHECK_EQUAL(outer.size(), 4);
    BOOST_CHECK_EQUAL(holes.size(), 0);
    BOOST_CHECK_EQUAL(outer[0].x, 1);
    BOOST_CHECK_EQUAL(outer[0].y, 1);
    BOOST_CHECK_EQUAL(outer[1].x, 1);
    BOOST_CHECK_EQUAL(outer[1].y, 5);
    BOOST_CHECK_EQUAL(outer[2].x, 5);
    BOOST_CHECK_EQUAL(outer[2].y, 5);
    BOOST_CHECK_EQUAL(outer[3].x, 5);
    BOOST_CHECK_EQUAL(outer[3].y, 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_2, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 2, 3);
    rgn.AddRun(1, 5, 6);
    rgn.AddRun(2, 1, 7);
    rgn.AddRun(3, 1, 3);
    rgn.AddRun(3, 5, 7);
    rgn.AddRun(4, 2, 3);
    rgn.AddRun(4, 5, 6);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 7);
    BOOST_CHECK_EQUAL(outers.size(), 1);
    BOOST_CHECK_EQUAL(outer.size(), 20);
    BOOST_CHECK_EQUAL(holes.size(), 0);
    BOOST_CHECK_EQUAL(outer[0].x, 2);
    BOOST_CHECK_EQUAL(outer[0].y, 1);
    BOOST_CHECK_EQUAL(outer[1].x, 2);
    BOOST_CHECK_EQUAL(outer[1].y, 2);
    BOOST_CHECK_EQUAL(outer[2].x, 1);
    BOOST_CHECK_EQUAL(outer[2].y, 2);
    BOOST_CHECK_EQUAL(outer[3].x, 1);
    BOOST_CHECK_EQUAL(outer[3].y, 4);
    BOOST_CHECK_EQUAL(outer[4].x, 2);
    BOOST_CHECK_EQUAL(outer[4].y, 4);
    BOOST_CHECK_EQUAL(outer[5].x, 2);
    BOOST_CHECK_EQUAL(outer[5].y, 5);
    BOOST_CHECK_EQUAL(outer[6].x, 3);
    BOOST_CHECK_EQUAL(outer[6].y, 5);
    BOOST_CHECK_EQUAL(outer[7].x, 3);
    BOOST_CHECK_EQUAL(outer[7].y, 3);
    BOOST_CHECK_EQUAL(outer[8].x, 5);
    BOOST_CHECK_EQUAL(outer[8].y, 3);
    BOOST_CHECK_EQUAL(outer[9].x, 5);
    BOOST_CHECK_EQUAL(outer[9].y, 5);
    BOOST_CHECK_EQUAL(outer[10].x, 6);
    BOOST_CHECK_EQUAL(outer[10].y, 5);
    BOOST_CHECK_EQUAL(outer[11].x, 6);
    BOOST_CHECK_EQUAL(outer[11].y, 4);
    BOOST_CHECK_EQUAL(outer[12].x, 7);
    BOOST_CHECK_EQUAL(outer[12].y, 4);
    BOOST_CHECK_EQUAL(outer[13].x, 7);
    BOOST_CHECK_EQUAL(outer[13].y, 2);
    BOOST_CHECK_EQUAL(outer[14].x, 6);
    BOOST_CHECK_EQUAL(outer[14].y, 2);
    BOOST_CHECK_EQUAL(outer[15].x, 6);
    BOOST_CHECK_EQUAL(outer[15].y, 1);
    BOOST_CHECK_EQUAL(outer[16].x, 5);
    BOOST_CHECK_EQUAL(outer[16].y, 1);
    BOOST_CHECK_EQUAL(outer[17].x, 5);
    BOOST_CHECK_EQUAL(outer[17].y, 2);
    BOOST_CHECK_EQUAL(outer[18].x, 3);
    BOOST_CHECK_EQUAL(outer[18].y, 2);
    BOOST_CHECK_EQUAL(outer[19].x, 3);
    BOOST_CHECK_EQUAL(outer[19].y, 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_3, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 1, 6);
    rgn.AddRun(2, 1, 2);
    rgn.AddRun(2, 5, 6);
    rgn.AddRun(3, 1, 2);
    rgn.AddRun(3, 5, 6);
    rgn.AddRun(4, 1, 6);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 6);
    BOOST_CHECK_EQUAL(outers.size(), 1);
    BOOST_CHECK_EQUAL(outer.size(), 4);
    BOOST_CHECK_EQUAL(holes.size(), 1);
    BOOST_CHECK_EQUAL(holes[0].size(), 4);
    BOOST_CHECK_EQUAL(outer[0].x, 1);
    BOOST_CHECK_EQUAL(outer[0].y, 1);
    BOOST_CHECK_EQUAL(outer[1].x, 1);
    BOOST_CHECK_EQUAL(outer[1].y, 5);
    BOOST_CHECK_EQUAL(outer[2].x, 6);
    BOOST_CHECK_EQUAL(outer[2].y, 5);
    BOOST_CHECK_EQUAL(outer[3].x, 6);
    BOOST_CHECK_EQUAL(outer[3].y, 1);
    BOOST_CHECK_EQUAL(holes[0][0].x, 5);
    BOOST_CHECK_EQUAL(holes[0][0].y, 2);
    BOOST_CHECK_EQUAL(holes[0][1].x, 5);
    BOOST_CHECK_EQUAL(holes[0][1].y, 4);
    BOOST_CHECK_EQUAL(holes[0][2].x, 2);
    BOOST_CHECK_EQUAL(holes[0][2].y, 4);
    BOOST_CHECK_EQUAL(holes[0][3].x, 2);
    BOOST_CHECK_EQUAL(holes[0][3].y, 2);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_4, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(1, 1, 7);
    rgn.AddRun(2, 1, 2);
    rgn.AddRun(2, 4, 5);
    rgn.AddRun(2, 6, 7);
    rgn.AddRun(3, 1, 2);
    rgn.AddRun(3, 4, 5);
    rgn.AddRun(3, 6, 7);
    rgn.AddRun(4, 1, 5);
    rgn.AddRun(4, 6, 7);
    rgn.AddRun(5, 1, 2);
    rgn.AddRun(5, 6, 7);
    rgn.AddRun(6, 1, 7);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_CHECK_EQUAL(rgn.GetNumRuns(), 12);
    BOOST_CHECK_EQUAL(outers.size(), 1);
    BOOST_CHECK_EQUAL(outer.size(), 4);
    BOOST_CHECK_EQUAL(holes.size(), 2);
    BOOST_CHECK_EQUAL(holes[0].size(), 4);
    BOOST_CHECK_EQUAL(holes[1].size(), 6);
    BOOST_CHECK_EQUAL(outer[0].x, 1);
    BOOST_CHECK_EQUAL(outer[0].y, 1);
    BOOST_CHECK_EQUAL(outer[1].x, 1);
    BOOST_CHECK_EQUAL(outer[1].y, 7);
    BOOST_CHECK_EQUAL(outer[2].x, 7);
    BOOST_CHECK_EQUAL(outer[2].y, 7);
    BOOST_CHECK_EQUAL(outer[3].x, 7);
    BOOST_CHECK_EQUAL(outer[3].y, 1);
    BOOST_CHECK_EQUAL(holes[0][0].x, 4);
    BOOST_CHECK_EQUAL(holes[0][0].y, 2);
    BOOST_CHECK_EQUAL(holes[0][1].x, 4);
    BOOST_CHECK_EQUAL(holes[0][1].y, 4);
    BOOST_CHECK_EQUAL(holes[0][2].x, 2);
    BOOST_CHECK_EQUAL(holes[0][2].y, 4);
    BOOST_CHECK_EQUAL(holes[0][3].x, 2);
    BOOST_CHECK_EQUAL(holes[0][3].y, 2);
    BOOST_CHECK_EQUAL(holes[1][0].x, 6);
    BOOST_CHECK_EQUAL(holes[1][0].y, 2);
    BOOST_CHECK_EQUAL(holes[1][1].x, 6);
    BOOST_CHECK_EQUAL(holes[1][1].y, 6);
    BOOST_CHECK_EQUAL(holes[1][2].x, 2);
    BOOST_CHECK_EQUAL(holes[1][2].y, 6);
    BOOST_CHECK_EQUAL(holes[1][3].x, 2);
    BOOST_CHECK_EQUAL(holes[1][3].y, 5);
    BOOST_CHECK_EQUAL(holes[1][4].x, 5);
    BOOST_CHECK_EQUAL(holes[1][4].y, 5);
    BOOST_CHECK_EQUAL(holes[1][5].x, 5);
    BOOST_CHECK_EQUAL(holes[1][5].y, 2);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_5, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(2, 2, 6);
    rgn.AddRun(2, 7, 11);
    rgn.AddRun(3, 2, 3);
    rgn.AddRun(3, 5, 6);
    rgn.AddRun(3, 7, 8);
    rgn.AddRun(3, 10, 11);
    rgn.AddRun(4, 2, 3);
    rgn.AddRun(4, 5, 6);
    rgn.AddRun(4, 7, 8);
    rgn.AddRun(4, 10, 11);
    rgn.AddRun(5, 2, 6);
    rgn.AddRun(5, 7, 11);
    rgn.AddRun(8, 2, 11);
    rgn.AddRun(9, 2, 3);
    rgn.AddRun(9, 5, 8);
    rgn.AddRun(9, 10, 11);
    rgn.AddRun(10, 2, 3);
    rgn.AddRun(10, 5, 8);
    rgn.AddRun(10, 10, 11);
    rgn.AddRun(11, 2, 11);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_REQUIRE_EQUAL(outers.size(), 3);
    BOOST_REQUIRE_EQUAL(holes.size(), 4);
    BOOST_REQUIRE_EQUAL(outers[0].size(), 4);
    BOOST_REQUIRE_EQUAL(outers[1].size(), 4);
    BOOST_REQUIRE_EQUAL(outers[2].size(), 4);
    BOOST_REQUIRE_EQUAL(holes[0].size(), 4);
    BOOST_REQUIRE_EQUAL(holes[1].size(), 4);
    BOOST_REQUIRE_EQUAL(holes[2].size(), 4);
    BOOST_REQUIRE_EQUAL(holes[3].size(), 4);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_6, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 3);
    rgn.AddRun(1, 0, 1);
    rgn.AddRun(1, 2, 3);
    rgn.AddRun(5, 0, 3);
    rgn.AddRun(6, 0, 1);
    rgn.AddRun(6, 2, 3);
    rgn.AddRun(7, 0, 3);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();

    BOOST_REQUIRE_EQUAL(outers.size(), 2);
    BOOST_REQUIRE_EQUAL(holes.size(), 1);
    BOOST_REQUIRE_EQUAL(outers[0].size(), 8);
    BOOST_REQUIRE_EQUAL(outers[1].size(), 4);
    BOOST_REQUIRE_EQUAL(holes[0].size(), 4);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_7, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    cv::Mat img = cv::Mat::zeros(200, 200, CV_8UC1);
    cv::putText(img, cv::String("Hello"), cv::Point(20, 80), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255), 3);
    cv::putText(img, cv::String("OpenGL"), cv::Point(20, 120), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255), 3);
    rgn.AddRun(img);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    const RD_CONTOUR &outer = outers.front();
    BOOST_REQUIRE_EQUAL(outers.size(), 9);
    BOOST_REQUIRE_EQUAL(holes.size(), 8);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_8, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    cv::Mat img = cv::Mat::zeros(200, 200, CV_8UC1);
    const cv::Point points[1][4] = { {cv::Point(20, 20), cv::Point(20, 100), cv::Point(100, 100), cv::Point(100, 20)}};
    const cv::Point *ppt[1] = { points[0]};
    const int npts[1] = { 4 };
    cv::fillPoly(img, &ppt[0], &npts[0], 1, cv::Scalar(255, 255, 255));
    rgn.AddRun(img);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    BOOST_REQUIRE_EQUAL(outers.size(), 1);
    BOOST_REQUIRE_EQUAL(outers[0].size(), 4);
    BOOST_REQUIRE_EQUAL(holes.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_9, *boost::unit_test::enable_if<vtune_build>())
{
    SpamRgn rgn;
    cv::Mat img = cv::Mat::zeros(200, 200, CV_8UC1);
    const cv::Point points[2][5] = { {cv::Point(20, 20), cv::Point(20, 100), cv::Point(60, 60), cv::Point(100, 100), cv::Point(100, 20)}, {cv::Point(60, 80), cv::Point(20, 140), cv::Point(100, 140)}};
    const cv::Point *ppt[2] = { points[0], points[1] };
    const int npts[2] = {5, 3};
    cv::fillPoly(img, &ppt[0], &npts[0], 2, cv::Scalar(255, 255, 255));
    rgn.AddRun(img);

    RunTypeDirectionEncoder encoder(rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    BOOST_REQUIRE_EQUAL(outers.size(), 2);
    BOOST_REQUIRE_EQUAL(holes.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_10, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("scrach.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    UnitTestHelper::Color color{ rgn->GetRed(), rgn->GetGreen(), rgn->GetBlue(), rgn->GetAlpha() };
    UnitTestHelper::DrawPathToImage(rgn->GetPath(), color, colorImg);
    UnitTestHelper::WriteImage(colorImg, "scrach.png");

    RunTypeDirectionEncoder encoder(*rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    BOOST_CHECK_EQUAL(outers.size(), 94);
    BOOST_CHECK_EQUAL(holes.size(), 36);

    SPSpamRgnVector rgns = rgn->Connect();
    BOOST_CHECK_EQUAL(rgns->size(), 94);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RowRanges_PERFORMANCE_0, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    tbb::tick_count t1 = tbb::tick_count::now();
    RowRunStartList rrl;
    rgn->GetRowRanges(rrl);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam get row ranges times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    BOOST_CHECK_EQUAL(rrl.size(), 2501);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Threshold_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    tbb::tick_count t1 = tbb::tick_count::now();
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam threshold times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(rgn->GetData().size(), 234794);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Run_Counter_Square, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg = cv::Mat::ones(3, 33, CV_8U) * 200;

    int numRuns = BasicImgProc::GetNumRunsOfBinaryImage(grayImg, 150, 255);
    BOOST_CHECK_EQUAL(numRuns, 3);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Run_Counter_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    tbb::tick_count t1 = tbb::tick_count::now();
    int numRuns = BasicImgProc::GetNumRunsOfBinaryImage(grayImg, 150, 255);
    tbb::tick_count t2 = tbb::tick_count::now();

    BOOST_TEST_MESSAGE("Spam rd runs counter times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(numRuns, 234794);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_Encode_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    tbb::tick_count t1 = tbb::tick_count::now();
    RunTypeDirectionEncoder encoder(*rgn);
    RD_LIST rdList = encoder.encode();
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam rd encode times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(rdList.size(), 469589);
    BOOST_CHECK_EQUAL(rdList.size(), rgn->GetData().size()*2+1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_TRACK_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    tbb::tick_count t1 = tbb::tick_count::now();
    RunTypeDirectionEncoder encoder(*rgn);
    RD_CONTOUR_LIST outers;
    RD_CONTOUR_LIST holes;
    encoder.track(outers, holes);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam track times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(outers.size(), 941);
    BOOST_CHECK_EQUAL(holes.size(), 17622);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Connect_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    tbb::tick_count t1 = tbb::tick_count::now();
    SPSpamRgnVector rgns = rgn->Connect();
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam connect times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(rgns->size(), 941);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Threshold_Mista_Draw, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");
    SPSpamRgn rgn = BasicImgProc::Threshold(grayImg, 150, 255);

    UnitTestHelper::Color color{ rgn->GetRed(), rgn->GetGreen(), rgn->GetBlue(), rgn->GetAlpha() };
    UnitTestHelper::DrawPathToImage(rgn->GetPath(), color, colorImg);
    UnitTestHelper::WriteImage(colorImg, "mista.png");

    BOOST_CHECK_EQUAL(rgn->GetData().size(), 234794);
}

BOOST_AUTO_TEST_CASE(test_AlignImageWidth, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Mat alignGrayImg = BasicImgProc::AlignImageWidth(grayImg);
    tbb::tick_count t2 = tbb::tick_count::now();

    BOOST_CHECK_EQUAL(alignGrayImg.step1()%64, 0);
    BOOST_TEST_MESSAGE("Align gray image width (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
}

BOOST_AUTO_TEST_CASE(test_Mvlab_Connect_Mista, *boost::unit_test::enable_if<vtune_build>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    cv::Ptr<cv::mvlab::Region> rgn;
    cv::mvlab::Threshold(grayImg, 150, 255, rgn);

    tbb::tick_count t1 = tbb::tick_count::now();
    cv::Ptr<cv::mvlab::RegionCollection> rgns;
    rgn->Connect(rgns, 8);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Spam connect times(mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(rgns->Count(), 941);
}