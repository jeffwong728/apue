import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata
import math
import random

class TestContourMinAreaRect(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        mvlab.SetGlobalOption('convex_hull_method', 'Andrew')
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_All_Points_Coincidence_MinAreaRect(self):
        points = [(300, 300)]*10
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 300.0)
        self.assertAlmostEqual(minRect[0][1], 300.0)

    def test_Identical_X_MinAreaRect(self):
        points = [(100, 100), (100, 200), (100, 300), (100, 400), (100, 500)]
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 100.0)
        self.assertAlmostEqual(minRect[0][1], 300.0)

    def test_Identical_Y_MinAreaRect(self):
        points = [(100, 100), (200, 100), (300, 100), (400, 100), (450, 100), (500, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 300.0)
        self.assertAlmostEqual(minRect[0][1], 100.0)

    def test_Triangle_MinAreaRect_CW(self):
        points = [(50, 200), (100, 100), (150, 200)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 100.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 100.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 180.0)

    def test_Triangle_MinAreaRect_CCW(self):
        points = [(50, 200), (150, 200), (100, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 115.0)
        self.assertAlmostEqual(minRect[0][1], 170.0)
        self.assertAlmostEqual(minRect[1][0], 111.80339813232422)
        self.assertAlmostEqual(minRect[1][1], 89.44271850585938)
        self.assertAlmostEqual(minRect[2], 116.5650405883789)

    def test_Trapezoid_MinAreaRect_CW(self):
        points = [(100, 100), (200, 100), (250, 200), (50, 200)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 150.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 200.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 0.0)

    def test_Trapezoid_MinAreaRect_CCW(self):
        points = [(100, 100), (50, 200), (250, 200), (200, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 150.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 200.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 180.0)

    def test_RotatedRectangle_MinAreaRect(self):
        contr = mvlab.Contour_GenRotatedRectangle(((100, 100), (50, 30), 30))
        minRect = contr.SmallestRectangle()

        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 100, delta=0.001)
        self.assertAlmostEqual(minRect[0][1], 100, delta=0.001)
        self.assertAlmostEqual(minRect[1][0], 50, delta=0.001)
        self.assertAlmostEqual(minRect[1][1], 30, delta=0.001)
        self.assertAlmostEqual(minRect[2], 210, delta=0.001)

    def test_RotatedEllipse_MinAreaRect_CW(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 2, 'negative')
        startTime = time.perf_counter()
        minRect = contr.SmallestRectangle()
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[0][1], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][1], 600, delta=0.1)
        self.assertAlmostEqual(minRect[2], 150, delta=0.1)

    def test_RotatedEllipse_MinAreaRect_CCW(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 2, 'positive')
        startTime = time.perf_counter()
        minRect = contr.SmallestRectangle()
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[0][1], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][1], 600, delta=0.1)
        self.assertAlmostEqual(minRect[2], 150, delta=0.1)

    def test_Convex_Polygon_MinAreaRect(self):
        points = [(250.22852, 16.357777), (444.14212, 1.1392639), (622.03546, 38.92456)]
        points += [(543.08356, 340.36865), (483.9272, 436.66464), (386.01123, 475.3857)]
        points += [(43.13343, 361.93884), (6.043603, 324.05008), (0.8927474, 42.904503), (8.817269, 38.88925)]
        points = [(x+150, y+150) for (x, y) in points]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 425.828857421875)
        self.assertAlmostEqual(minRect[0][1], 358.6856689453125)
        self.assertAlmostEqual(minRect[1][0], 606.7611083984375)
        self.assertAlmostEqual(minRect[1][1], 475.9751892089844)
        self.assertAlmostEqual(minRect[2], 11.991644859313965)

    def test_Random_Small_MinAreaRect(self):
        n = int(random.uniform(1, 30))
        points = []
        for i in range(0, n):
            points.append((random.uniform(150, 640), random.uniform(150, 480)))
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        eMinRect = mvlab.Contour_GenRotatedRectangle(((minRect[0][0], minRect[0][1]), (minRect[1][0]+1, minRect[1][1]+1), minRect[2]))
        for point in points:
            if not eMinRect.TestPoint(point):
                print(points)
            self.assertTrue(eMinRect.TestPoint(point))

    def test_Random_Large_MinAreaRect(self):
        n = int(random.uniform(1, 300))
        points = []
        for i in range(0, n):
            points.append((random.uniform(150, 640), random.uniform(150, 480)))
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        eMinRect = mvlab.Contour_GenRotatedRectangle(((minRect[0][0], minRect[0][1]), (minRect[1][0]+1, minRect[1][1]+1), minRect[2]))
        for point in points:
            if not eMinRect.TestPoint(point):
                print(points)
            self.assertTrue(eMinRect.TestPoint(point))

if __name__ == '__main__':
    unittest.main()
