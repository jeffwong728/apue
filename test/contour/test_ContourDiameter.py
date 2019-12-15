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

class TestContourDiameter(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')
    @staticmethod
    def diameter(d):
        return math.sqrt((d[0]-d[2])*(d[0]-d[2])+(d[1]-d[3])*(d[1]-d[3]))

    def test_Simple_Convex(self):
        contr = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (150, 150), (200, 200), (200, 100)])
        d = contr.Diameter()
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Circle_Diameter(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 5, 'negative')
        startTime = time.perf_counter()
        d = contr.Diameter()
        endTime = time.perf_counter()
        print(contr.CountPoints())
        self.assertAlmostEqual(TestContourDiameter.diameter(d), 1000, delta=0.001)
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_CircleArc_Diameter(self):
        contr = mvlab.Contour_GenCircleSector((1000, 1000), 500, 10, 300, 5, 'negative slice')
        startTime = time.perf_counter()
        d = contr.Diameter()
        endTime = time.perf_counter()
        print(contr.CountPoints())
        self.assertAlmostEqual(TestContourDiameter.diameter(d), 1000, delta=0.001)
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Ellipse_Diameter(self):
        contr = mvlab.Contour_GenEllipse((1000, 1000), (500, 300), 5, 'negative')
        startTime = time.perf_counter()
        d = contr.Diameter()
        endTime = time.perf_counter()
        print(contr.CountPoints())
        self.assertAlmostEqual(TestContourDiameter.diameter(d), 1000, delta=0.1)
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_RotatedEllipse_Diameter(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 5, 'positive')
        startTime = time.perf_counter()
        d = contr.Diameter()
        endTime = time.perf_counter()
        print(contr.CountPoints())
        self.assertAlmostEqual(TestContourDiameter.diameter(d), 1000, delta=0.1)
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Diameter_Coincidence(self):
        contr = mvlab.Contour_GenPolygon([(150, 100), (100, 100), (100, 150), (100, 200), (100, 200), (150, 200), (200, 200), (200, 150), (200, 100)])
        d = contr.Diameter()
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Diameter_CCW(self):
        contr = mvlab.Contour_GenPolygon([(150, 150), (200, 200), (200, 150), (200, 100), (100, 100), (100, 100), (100, 200)])
        d = contr.Diameter()
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Diameter_CW(self):
        contr = mvlab.Contour_GenPolygon([(100, 200), (100, 100), (100, 100), (200, 100), (200, 150), (200, 200), (150, 150)])
        d = contr.Diameter()
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [contr, pointConts, dl])

    def test_Random_Polygon(self):
        n = int(random.uniform(1, 100))
        points = []
        for i in range(0, n):
            points.append((random.uniform(0, 640), random.uniform(0, 480)))

        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        d = plg.Diameter()
        points = [(d[0], d[1]), (d[2], d[3])]
        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*2, [0]*2)
        dl = mvlab.Contour_GenPolyline(points)
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), pointConts, dl])

if __name__ == '__main__':
    unittest.main()
