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

class TestContourArea(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Contour_Area(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        startTime = time.perf_counter()
        a = contr.Area()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertAlmostEqual(a, math.pi*500*500, delta=0.6)

        contr = mvlab.Contour_GenEmpty()
        self.assertAlmostEqual(contr.Area(), 0)

        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Area(), 100*100)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Area(), 4)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1), (1.5, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Area(), 4)

        points = [(8.817269 , 38.88925), (250.22852 , 16.357777), (444.14212 , 1.1392639)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Area(), 347.61962890625)

    def test_Contour_Centroid(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        startTime = time.perf_counter()
        x, y = contr.GetCentroid()[0]
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertAlmostEqual(x, 1000, delta=0.001)
        self.assertAlmostEqual(y, 1000, delta=0.001)

        contr = mvlab.Contour_GenEmpty()
        x, y = contr.GetCentroid()[0]
        self.assertAlmostEqual(x, 0)
        self.assertAlmostEqual(y, 0)

        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.GetCentroid()[0]
        self.assertAlmostEqual(x, 150)
        self.assertAlmostEqual(y, 150)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.GetCentroid()[0]
        self.assertAlmostEqual(x, 2)
        self.assertAlmostEqual(y, 2)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1), (1.5, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.GetCentroid()[0]
        self.assertAlmostEqual(x, 2)
        self.assertAlmostEqual(y, 2)

    def test_Equilateral_Area(self):
        contrs = []
        for step in [10, 20, 30, 40, 60, 90]:
            points = [(300, 200)]
            for angle in range(step, 360, step):
                affMat = mvlab.HomoMat2d_GenIdentity()
                affMat = mvlab.HomoMat2d_Rotate(affMat, angle, (200, 200))
                points.append(mvlab.HomoMat2d_AffineTransPoint2d(affMat, (300, 200)))
            contr = mvlab.Contour_GenPolygon(points)
            contrs.append(contr)
            c = math.sqrt(100*100+100*100-2*100*100*math.cos(step*math.pi/180))
            h = 100*math.cos(step/2*math.pi/180)
            self.assertAlmostEqual(contr.Area(), 360/step*c*h/2, delta=0.01, msg='step={0:d}'.format(step))

            x, y = contr.Centroid()
            self.assertAlmostEqual(x, 200, delta=0.001)
            self.assertAlmostEqual(y, 200, delta=0.001)
        extradata.SaveContours(self.id(), contrs)

    def test_Segment_Area(self):
        points = [(100, 100), (150, 150), (200, 200)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Area(), 0)

        x, y = contr.Centroid()
        self.assertAlmostEqual(x, 150)
        self.assertAlmostEqual(y, 150)

if __name__ == '__main__':
    unittest.main()
