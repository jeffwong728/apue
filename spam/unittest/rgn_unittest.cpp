#define BOOST_TEST_MODULE TestSpamRgn
#include <boost/test/included/unit_test.hpp>
#include <ui/proc/rgn.h>

BOOST_AUTO_TEST_CASE(test_SpamRgn_0_Area)
{
    SpamRgn rgn;
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 0.0, 0.1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_1_Area)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 1);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 1.0, 0.1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Area)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 1);
    rgn.AddRun(0, 3, 10);
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 8.0, 0.1);

    rgn.clear();
    BOOST_REQUIRE_CLOSE_FRACTION(rgn.Area(), 0.0, 0.1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_Empty)
{
    SpamRgn rgn;
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_1_Row_0)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 1);
    BOOST_REQUIRE_EQUAL(al[0].size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_1_Row_1)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 7, 10);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 2);
    BOOST_REQUIRE_EQUAL(al[0].size(), 0);
    BOOST_REQUIRE_EQUAL(al[1].size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_1_Row_2)
{
    SpamRgn rgn;
    rgn.AddRun(1, 0, 5);
    rgn.AddRun(1, 7, 10);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 2);
    BOOST_REQUIRE_EQUAL(al[0].size(), 0);
    BOOST_REQUIRE_EQUAL(al[1].size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_1_Row_3)
{
    SpamRgn rgn;
    rgn.AddRun(2, 0, 5);
    rgn.AddRun(2, 7, 10);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 2);
    BOOST_REQUIRE_EQUAL(al[0].size(), 0);
    BOOST_REQUIRE_EQUAL(al[1].size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_2_Row_1)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 7, 10);
    rgn.AddRun(1, 0, 5);
    rgn.AddRun(1, 7, 10);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 4);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 1);
    BOOST_REQUIRE_EQUAL(al[2].size(), 1);
    BOOST_REQUIRE_EQUAL(al[3].size(), 1);
    BOOST_REQUIRE_EQUAL(al[0][0], 2);
    BOOST_REQUIRE_EQUAL(al[1][0], 3);
    BOOST_REQUIRE_EQUAL(al[2][0], 0);
    BOOST_REQUIRE_EQUAL(al[3][0], 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_2_Row_2)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 7, 10);
    rgn.AddRun(1, 5, 7);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 3);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 1);
    BOOST_REQUIRE_EQUAL(al[2].size(), 2);
    BOOST_REQUIRE_EQUAL(al[0][0], 2);
    BOOST_REQUIRE_EQUAL(al[1][0], 2);
    BOOST_REQUIRE_EQUAL(al[2][0], 0);
    BOOST_REQUIRE_EQUAL(al[2][1], 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_2_Row_3)
{
    SpamRgn rgn;
    rgn.AddRun(1, 0, 5);
    rgn.AddRun(1, 7, 10);
    rgn.AddRun(2, 5, 7);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 3);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 1);
    BOOST_REQUIRE_EQUAL(al[2].size(), 2);
    BOOST_REQUIRE_EQUAL(al[0][0], 2);
    BOOST_REQUIRE_EQUAL(al[1][0], 2);
    BOOST_REQUIRE_EQUAL(al[2][0], 0);
    BOOST_REQUIRE_EQUAL(al[2][1], 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_2_Row_4)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 7, 10);
    rgn.AddRun(2, 5, 7);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 3);
    BOOST_REQUIRE_EQUAL(al[0].size(), 0);
    BOOST_REQUIRE_EQUAL(al[1].size(), 0);
    BOOST_REQUIRE_EQUAL(al[2].size(), 0);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_2_Row_5)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 7, 10);
    rgn.AddRun(0, 15, 20);
    rgn.AddRun(0, 25, 30);
    rgn.AddRun(0, 35, 40);
    rgn.AddRun(0, 45, 50);
    rgn.AddRun(1, 1, 70);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 7);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 1);
    BOOST_REQUIRE_EQUAL(al[2].size(), 1);
    BOOST_REQUIRE_EQUAL(al[3].size(), 1);
    BOOST_REQUIRE_EQUAL(al[4].size(), 1);
    BOOST_REQUIRE_EQUAL(al[5].size(), 1);
    BOOST_REQUIRE_EQUAL(al[6].size(), 6);
    BOOST_REQUIRE_EQUAL(al[0][0], 6);
    BOOST_REQUIRE_EQUAL(al[1][0], 6);
    BOOST_REQUIRE_EQUAL(al[2][0], 6);
    BOOST_REQUIRE_EQUAL(al[3][0], 6);
    BOOST_REQUIRE_EQUAL(al[4][0], 6);
    BOOST_REQUIRE_EQUAL(al[5][0], 6);
    BOOST_REQUIRE_EQUAL(al[6][0], 0);
    BOOST_REQUIRE_EQUAL(al[6][1], 1);
    BOOST_REQUIRE_EQUAL(al[6][2], 2);
    BOOST_REQUIRE_EQUAL(al[6][3], 3);
    BOOST_REQUIRE_EQUAL(al[6][4], 4);
    BOOST_REQUIRE_EQUAL(al[6][5], 5);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_3_Row_1)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(1, 3, 10);
    rgn.AddRun(2, 5, 7);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 3);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 2);
    BOOST_REQUIRE_EQUAL(al[2].size(), 1);
    BOOST_REQUIRE_EQUAL(al[0][0], 1);
    BOOST_REQUIRE_EQUAL(al[1][0], 0);
    BOOST_REQUIRE_EQUAL(al[1][1], 2);
    BOOST_REQUIRE_EQUAL(al[2][0], 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_3_Row_2)
{
    SpamRgn rgn;
    rgn.AddRun(1, 0, 5);
    rgn.AddRun(2, 3, 10);
    rgn.AddRun(3, 5, 7);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 3);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 2);
    BOOST_REQUIRE_EQUAL(al[2].size(), 1);
    BOOST_REQUIRE_EQUAL(al[0][0], 1);
    BOOST_REQUIRE_EQUAL(al[1][0], 0);
    BOOST_REQUIRE_EQUAL(al[1][1], 2);
    BOOST_REQUIRE_EQUAL(al[2][0], 1);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_AdjacencyList_3_Row_3)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 5);
    rgn.AddRun(0, 10, 15);
    rgn.AddRun(0, 20, 25);
    rgn.AddRun(1, 1, 30);
    rgn.AddRun(2, 0, 5);
    rgn.AddRun(2, 10, 15);
    rgn.AddRun(2, 20, 25);
    AdjacencyList al = rgn.GetAdjacencyList();
    BOOST_REQUIRE_EQUAL(al.size(), 7);
    BOOST_REQUIRE_EQUAL(al[0].size(), 1);
    BOOST_REQUIRE_EQUAL(al[1].size(), 1);
    BOOST_REQUIRE_EQUAL(al[2].size(), 1);
    BOOST_REQUIRE_EQUAL(al[3].size(), 6);
    BOOST_REQUIRE_EQUAL(al[4].size(), 1);
    BOOST_REQUIRE_EQUAL(al[5].size(), 1);
    BOOST_REQUIRE_EQUAL(al[6].size(), 1);
    BOOST_REQUIRE_EQUAL(al[0][0], 3);
    BOOST_REQUIRE_EQUAL(al[1][0], 3);
    BOOST_REQUIRE_EQUAL(al[2][0], 3);
    BOOST_REQUIRE_EQUAL(al[3][0], 0);
    BOOST_REQUIRE_EQUAL(al[3][1], 1);
    BOOST_REQUIRE_EQUAL(al[3][2], 2);
    BOOST_REQUIRE_EQUAL(al[3][3], 4);
    BOOST_REQUIRE_EQUAL(al[3][4], 5);
    BOOST_REQUIRE_EQUAL(al[3][5], 6);
    BOOST_REQUIRE_EQUAL(al[4][0], 3);
    BOOST_REQUIRE_EQUAL(al[5][0], 3);
    BOOST_REQUIRE_EQUAL(al[6][0], 3);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_Connect_Rgn_2)
{
    SpamRgn rgn;
    rgn.AddRun(0, 0, 10);
    rgn.AddRun(0, 20, 30);
    rgn.AddRun(1, 0, 5);
    rgn.AddRun(1, 7, 10);
    rgn.AddRun(1, 20, 25);
    rgn.AddRun(1, 27, 30);
    rgn.AddRun(2, 0, 10);
    rgn.AddRun(2, 20, 30);

    SPSpamRgnVector rgns = rgn.Connect();
    BOOST_REQUIRE_EQUAL(rgns->size(), 2);
}

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_0)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_1)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_2)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_3)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_4)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_5)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_6)
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

BOOST_AUTO_TEST_CASE(test_SpamRgn_RD_LIST_7)
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