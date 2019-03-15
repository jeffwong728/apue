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