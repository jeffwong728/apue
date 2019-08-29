#define BOOST_TEST_MODULE TestPixelTmpl
#include "helper.h"
#include <cassert>
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
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

BOOST_AUTO_TEST_CASE(test_PixelTmpl_CandidateRun_Empty)
{
    PixelTemplate::CandidateList cl;
    PixelTemplate::CandidateGroup cg(cl);
    BOOST_CHECK_EQUAL(cg.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_CandidateRun_Single)
{
    PixelTemplate::CandidateList cl;
    cl.emplace_back(1, 1, 239.f);
    PixelTemplate::CandidateGroup cg(cl);
    BOOST_CHECK_EQUAL(cg.size(), 1);
    BOOST_CHECK_EQUAL(cg.front().row, 1);
    BOOST_CHECK_EQUAL(cg.front().colb, 1);
    BOOST_CHECK_EQUAL(cg.front().cole, 2);
    BOOST_CHECK_CLOSE(cg.front().best.score, 239.f, 1e-9);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_CandidateRun_MultipleCandidates)
{
    PixelTemplate::CandidateList cl;
    cl.emplace_back(1, 1, 200.f);
    cl.emplace_back(1, 2, 239.f);
    PixelTemplate::CandidateGroup cg(cl);
    BOOST_CHECK_EQUAL(cg.size(), 1);
    BOOST_CHECK_EQUAL(cg.front().row, 1);
    BOOST_CHECK_EQUAL(cg.front().colb, 1);
    BOOST_CHECK_EQUAL(cg.front().cole, 3);
    BOOST_CHECK_CLOSE(cg.front().best.score, 239.f, 1e-9);

    cl.emplace_back(1, 3, 220.f);
    cg.RLEncodeCandidates(cl);
    BOOST_CHECK_EQUAL(cg.size(), 1);
    BOOST_CHECK_EQUAL(cg.front().row, 1);
    BOOST_CHECK_EQUAL(cg.front().colb, 1);
    BOOST_CHECK_EQUAL(cg.front().cole, 4);
    BOOST_CHECK_CLOSE(cg.front().best.score, 239.f, 1e-9);

    cl.emplace_back(1, 5, 201.f);
    cg.RLEncodeCandidates(cl);
    BOOST_CHECK_EQUAL(cg.size(), 2);
    BOOST_CHECK_EQUAL(cg.front().row, 1);
    BOOST_CHECK_EQUAL(cg.front().colb, 1);
    BOOST_CHECK_EQUAL(cg.front().cole, 4);
    BOOST_CHECK_CLOSE(cg.front().best.score, 239.f, 1e-9);
    BOOST_CHECK_EQUAL(cg.back().row, 1);
    BOOST_CHECK_EQUAL(cg.back().colb, 5);
    BOOST_CHECK_EQUAL(cg.back().cole, 6);
    BOOST_CHECK_CLOSE(cg.back().best.score, 201.f, 1e-9);

    cl.emplace_back(2, 1, 100.f);
    cg.RLEncodeCandidates(cl);
    BOOST_CHECK_EQUAL(cg.size(), 3);
    BOOST_CHECK_EQUAL(cg[0].row, 1);
    BOOST_CHECK_EQUAL(cg[0].colb, 1);
    BOOST_CHECK_EQUAL(cg[0].cole, 4);
    BOOST_CHECK_CLOSE(cg[0].best.score, 239.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[1].row, 1);
    BOOST_CHECK_EQUAL(cg[1].colb, 5);
    BOOST_CHECK_EQUAL(cg[1].cole, 6);
    BOOST_CHECK_CLOSE(cg[1].best.score, 201.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[2].row, 2);
    BOOST_CHECK_EQUAL(cg[2].colb, 1);
    BOOST_CHECK_EQUAL(cg[2].cole, 2);
    BOOST_CHECK_CLOSE(cg[2].best.score, 100.f, 1e-9);

    cl.emplace_back(2, 2, 101.f);
    cg.RLEncodeCandidates(cl);
    BOOST_CHECK_EQUAL(cg.size(), 3);
    BOOST_CHECK_EQUAL(cg[0].row, 1);
    BOOST_CHECK_EQUAL(cg[0].colb, 1);
    BOOST_CHECK_EQUAL(cg[0].cole, 4);
    BOOST_CHECK_CLOSE(cg[0].best.score, 239.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[1].row, 1);
    BOOST_CHECK_EQUAL(cg[1].colb, 5);
    BOOST_CHECK_EQUAL(cg[1].cole, 6);
    BOOST_CHECK_CLOSE(cg[1].best.score, 201.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[2].row, 2);
    BOOST_CHECK_EQUAL(cg[2].colb, 1);
    BOOST_CHECK_EQUAL(cg[2].cole, 3);
    BOOST_CHECK_CLOSE(cg[2].best.score, 101.f, 1e-9);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_CandidateRun_MultipleRuns)
{
    PixelTemplate::CandidateList cl;
    cl.emplace_back(1, 1, 250.f);
    cl.emplace_back(1, 2, 230.f);
    cl.emplace_back(1, 5, 200.f);
    cl.emplace_back(2, 3, 233.f);
    cl.emplace_back(2, 4, 251.f);
    cl.emplace_back(2, 5, 252.f);
    cl.emplace_back(3, 7, 211.f);
    PixelTemplate::CandidateGroup cg(cl);

    BOOST_CHECK_EQUAL(cg.size(), 4);
    BOOST_CHECK_EQUAL(cg[0].row, 1);
    BOOST_CHECK_EQUAL(cg[0].colb, 1);
    BOOST_CHECK_EQUAL(cg[0].cole, 3);
    BOOST_CHECK_CLOSE(cg[0].best.score, 250.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[1].row, 1);
    BOOST_CHECK_EQUAL(cg[1].colb, 5);
    BOOST_CHECK_EQUAL(cg[1].cole, 6);
    BOOST_CHECK_CLOSE(cg[1].best.score, 200.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[2].row, 2);
    BOOST_CHECK_EQUAL(cg[2].colb, 3);
    BOOST_CHECK_EQUAL(cg[2].cole, 6);
    BOOST_CHECK_CLOSE(cg[2].best.score, 252.f, 1e-9);
    BOOST_CHECK_EQUAL(cg[3].row, 3);
    BOOST_CHECK_EQUAL(cg[3].colb, 7);
    BOOST_CHECK_EQUAL(cg[3].cole, 8);
    BOOST_CHECK_CLOSE(cg[3].best.score, 211.f, 1e-9);
}

BOOST_AUTO_TEST_CASE(test_AngleInterval)
{
    BOOST_CHECK_EQUAL(AngleRange(0, 10).contains(5), true);
    BOOST_CHECK_EQUAL(AngleRange(-10, 10).contains(5), true);
    BOOST_CHECK_EQUAL(AngleRange(-10, 10).contains(-5), true);
    BOOST_CHECK_EQUAL(AngleRange(350, 10).contains(-5), true);
    BOOST_CHECK_EQUAL(AngleRange(10, 350).contains(-5), false);
    BOOST_CHECK_EQUAL(AngleRange(90, -90).contains(180), true);
    BOOST_CHECK_EQUAL(AngleRange(-90, 90).contains(180), false);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(180), true);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(-180), true);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(-200), false);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(200), false);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(-190), true);
    BOOST_CHECK_EQUAL(AngleRange(-190, -170).contains(-170), true);
    BOOST_CHECK_EQUAL(AngleRange(170, 190).contains(180), true);
    BOOST_CHECK_EQUAL(AngleRange(170, 190).contains(-180), true);
    BOOST_CHECK_EQUAL(AngleRange(170, 190).contains(160), false);
    BOOST_CHECK_EQUAL(AngleRange(170, 190).contains(-160), false);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_Create_Small, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(143, 121), Geom::Point(465, 177));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    tmplRgn.push_back(Geom::Path(Geom::Rect(Geom::Point(140, 311), Geom::Point(463, 368))));
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, 0, 5, 4, cv::TM_CCOEFF_NORMED };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerTemplData> &tmplDatas = pixelTmpl.GetTmplDatas();
    for (int l=0; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerTemplData &ltd = tmplDatas[l];
        const auto &tmplDatas = boost::get<BFNCCTemplDatas>(ltd.tmplDatas);
        for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
        {
            int i = 0;
            const BruteForceNCCTemplData &ptd = tmplDatas[t];
            const int width = ptd.maxPoint.x - ptd.minPoint.x + 1;
            const int height = ptd.maxPoint.y - ptd.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(128, 128, 128));
            for (const cv::Point &pt : ptd.locs)
            {
                tmplMat.at<uint8_t>(pt - ptd.minPoint) = static_cast<uint8_t>(ptd.vals[i++]);
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
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerTemplData> &tmplDatas = pixelTmpl.GetTmplDatas();
    for (int l = 3; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerTemplData &ltd = tmplDatas[l];
        const auto &tmplDatas = boost::get<PixelTemplDatas>(ltd.tmplDatas);
        for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
        {
            int i = 0;
            const PixelTemplData &ptd = tmplDatas[t];
            const int width = ptd.maxPoint.x - ptd.minPoint.x + 1;
            const int height = ptd.maxPoint.y - ptd.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(128, 128, 128));
            for (const cv::Point &pt : ptd.pixlLocs)
            {
                tmplMat.at<uint8_t>(pt - ptd.minPoint) = static_cast<uint8_t>(ptd.pixlVals[i++]);
            }

            std::string fileName = std::string("mista_tmpl_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
            UnitTestHelper::WriteImage(tmplMat, fileName);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_NCCTmpl_Create_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2200, 2050));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, 0, 10, 5, cv::TM_CCOEFF_NORMED };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create ncc template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    const std::vector<LayerTemplData> &tmplDatas = pixelTmpl.GetTmplDatas();
    for (int l = 0; l < static_cast<int>(tmplDatas.size()); ++l)
    {
        const LayerTemplData &ltd = tmplDatas[l];
        const auto &tmplDatas = boost::get<BFNCCTemplDatas>(ltd.tmplDatas);
        for (int t = 0; t < static_cast<int>(tmplDatas.size()); ++t)
        {
            int i = 0;
            const BruteForceNCCTemplData &ptd = tmplDatas[t];
            const int width = ptd.maxPoint.x - ptd.minPoint.x + 1;
            const int height = ptd.maxPoint.y - ptd.minPoint.y + 1;
            cv::Mat tmplMat(height, width, CV_8UC1, cv::Scalar(128, 128, 128));
            for (const cv::Point &pt : ptd.locs)
            {
                uint32_t typeVals[3]{0xFFFF0000, 0xFFFFFFFF, 0xFF00FF00};
                tmplMat.at<uint8_t>(pt - ptd.minPoint) = static_cast<uint8_t>(ptd.vals[i++]);
            }

            std::string fileName = std::string("mista_ncc_layer_") + std::to_string(l) + std::string("_number_") + std::to_string(t) + ".png";
            UnitTestHelper::WriteImage(tmplMat, fileName);
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_SAD_Small, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(185, 198), Geom::Point(399, 286));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -180, 359, 5, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    boost::filesystem::path utRootDir = std::getenv("SPAM_UNITTEST_ROOT");
    utRootDir.append("idata");
    utRootDir.append("images");
    utRootDir.append("board");

    boost::filesystem::path baseDir = std::getenv("SPAM_UNITTEST_ROOT");
    baseDir.append("idata");
    for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(utRootDir))
    {
        if (boost::filesystem::is_regular_file(x.path()) && x.path().extension()==".png")
        {
            cv::Point2f pos;
            float angle = 0;
            std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(boost::filesystem::relative(x.path(), baseDir).string());

            t1 = tbb::tick_count::now();
            sr = pixelTmpl.matchPixelTemplate(grayImg, 20, pos, angle);
            t2 = tbb::tick_count::now();
            BOOST_TEST_MESSAGE(std::string("Match pixel template (")+x.path().filename().string() +")" << (t2 - t1).seconds() * 1000 << "ms");

            std::string fileName = std::string("top_layer_")+x.path().filename().string();
            UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = pixelTmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_SAD_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2200, 2050));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, 0, 6, 5, cv::TM_SQDIFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0;

    t1 = tbb::tick_count::now();
    sr = pixelTmpl.matchPixelTemplate(grayImg, 20, pos, angle);
    t2 = tbb::tick_count::now();
    cv::Point2f pt = pixelTmpl.GetCenter();
    BOOST_CHECK_CLOSE(pos.x, pt.x, 1e-3f);
    BOOST_CHECK_CLOSE(pos.y, pt.y, 1e-3f);
    BOOST_CHECK_CLOSE(angle, 0.26284039f, 1e-3f);
    BOOST_TEST_MESSAGE("Match pixel template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::string fileName = std::string("mista_tmpl_top_layer_score.png");
    UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);

    if (SpamResult::kSR_SUCCESS == sr)
    {
        UnitTestHelper::Color color{ 255, 0, 0, 255 };
        const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
        UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
        UnitTestHelper::WriteImage(colorImg, std::string("mista_tmpl_match.png"));
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_NCC_Big, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::PathVector roi;
    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2200, 2050));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -60, 120, 5, cv::TM_CCOEFF_NORMED };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create ncc template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0, score=0;

    t1 = tbb::tick_count::now();
    sr = pixelTmpl.matchNCCTemplate(grayImg, 0.9f, pos, angle, score);
    t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Match ncc template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::string fileName = std::string("mista_ncc_top_layer_score.png");
    UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_NCC_Small, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("images\\board\\board-01.png");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(185, 198), Geom::Point(399, 286));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -180, 359, 5, cv::TM_CCOEFF_NORMED };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create brute force template (board-01.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

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
            sr = pixelTmpl.matchNCCTemplate(grayImg, 0.8f, pos, angle, score);
            t2 = tbb::tick_count::now();
            cv::Point2f pt = pixelTmpl.GetCenter();
            BOOST_TEST_MESSAGE(std::string("Match brute force ncc template (") + x.path().filename().string() + ") " \
                << (t2 - t1).seconds() * 1000 << "ms (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);

            std::string fileName = std::string("top_layer_") + x.path().filename().string();
            UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = pixelTmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_NCC_Gaskets, *boost::unit_test::enable_if<false>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("gasket\\filename000.jpg");

    const Geom::PathVector roi;
    const Geom::Rect rect1(Geom::Point(163, 197), Geom::Point(340, 340));
    const Geom::Path pth(rect1);
    Geom::PathVector tmplRgn(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, 0, 360, 5, cv::TM_CCOEFF_NORMED };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create brute force gasket template: " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    boost::filesystem::path utRootDir = std::getenv("SPAM_UNITTEST_ROOT");
    utRootDir.append("idata");
    utRootDir.append("gasket");

    boost::filesystem::path baseDir = std::getenv("SPAM_UNITTEST_ROOT");
    baseDir.append("idata");
    for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(utRootDir))
    {
        if (boost::filesystem::is_regular_file(x.path()) && x.path().extension() == ".jpg")
        {
            cv::Point2f pos;
            float angle = 0, score = 0;
            std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage(boost::filesystem::relative(x.path(), baseDir).string());

            t1 = tbb::tick_count::now();
            sr = pixelTmpl.matchNCCTemplate(grayImg, 0.65f, pos, angle, score);
            t2 = tbb::tick_count::now();
            cv::Point2f pt = pixelTmpl.GetCenter();
            BOOST_TEST_MESSAGE(std::string("Match brute force ncc template (") + x.path().filename().string() + ") " \
                << (t2 - t1).seconds() * 1000 << "ms (" << pos.x << ", " << pos.y << ", " << angle << "), with score=" << score);

            std::string fileName = std::string("top_layer_") + x.path().filename().string();
            UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);

            if (SpamResult::kSR_SUCCESS == sr)
            {
                UnitTestHelper::Color color{ 255, 0, 0, 255 };
                cv::Point2f pt = pixelTmpl.GetCenter();
                const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
                UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
                UnitTestHelper::WriteImage(colorImg, std::string("match_") + x.path().filename().string());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(test_PixelTmpl_BFNCC_Big, *boost::unit_test::enable_if<true>())
{
    cv::Mat grayImg, colorImg;
    std::tie(grayImg, colorImg) = UnitTestHelper::GetGrayScaleImage("mista.png");

    const Geom::Rect rect(Geom::Point(2000, 1850), Geom::Point(2300, 2150));
    const Geom::Path pth(rect);
    const Geom::PathVector tmplRgn(pth);
    const Geom::PathVector roi(pth);
    PixelTmplCreateData tmplCreateData{ grayImg , tmplRgn, roi, -1, 2, 6, cv::TM_CCOEFF };

    tbb::tick_count t1 = tbb::tick_count::now();
    PixelTemplate pixelTmpl;
    SpamResult sr = pixelTmpl.CreateTemplate(tmplCreateData);
    tbb::tick_count t2 = tbb::tick_count::now();
    BOOST_TEST_MESSAGE("Create bruteforce ncc template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");
    BOOST_CHECK_EQUAL(static_cast<long>(sr), static_cast<long>(SpamResult::kSR_SUCCESS));

    cv::Point2f pos;
    float angle = 0, score=0;

    t1 = tbb::tick_count::now();
    sr = pixelTmpl.matchNCCTemplate(grayImg, 0.9f, pos, angle, score);
    t2 = tbb::tick_count::now();
    cv::Point2f pt = pixelTmpl.GetCenter();
    BOOST_TEST_MESSAGE("Match bruteforce ncc template (mista.png): " << (t2 - t1).seconds() * 1000 << "ms");

    std::string fileName = std::string("mista_bruteforce_ncc_top_layer_score.png");
    UnitTestHelper::WriteImage(pixelTmpl.GetTopScoreMat(), fileName);

    if (SpamResult::kSR_SUCCESS == sr)
    {
        UnitTestHelper::Color color{ 255, 0, 0, 255 };
        const Geom::PathVector foundPV(tmplRgn * Geom::Translate(-pt.x, -pt.y) * Geom::Rotate::from_degrees(-angle)*Geom::Translate(pos.x, pos.y));
        UnitTestHelper::DrawPathToImage(foundPV, color, colorImg);
        UnitTestHelper::WriteImage(colorImg, std::string("mista_tmpl_match.png"));
        BOOST_TEST_MESSAGE("Template (mista.png) matched at: (" << pos.x << ", " << pos.y <<", "<< angle << "), with score=" << score);
    }
}