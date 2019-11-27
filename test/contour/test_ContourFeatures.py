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

class TestContourFeatures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Contour_BoundingBox(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.Count()))

        startTime = time.perf_counter()
        x, y, w, h = contr.BoundingBox()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)

        self.assertEqual(x, 500)
        self.assertEqual(y, 500)
        self.assertEqual(w, 1000)
        self.assertEqual(h, 1000)

        contr = mvlab.Contour_GenEmpty()
        x, y, w, h = contr.BoundingBox()
        self.assertEqual(x, 0)
        self.assertEqual(y, 0)
        self.assertEqual(w, 0)
        self.assertEqual(h, 0)

        points = [(100, 100), (50, 200), (150, 150), (250, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y, w, h = contr.BoundingBox()
        self.assertEqual(x, 50)
        self.assertEqual(y, 100)
        self.assertEqual(w, 200)
        self.assertEqual(h, 100)

    def test_Contour_Length(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.Count()))
        startTime = time.perf_counter()
        len = contr.Length()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertAlmostEqual(len, 2*math.pi*500, delta=0.001)

        contr = mvlab.Contour_GenEmpty()
        self.assertAlmostEqual(contr.Length(), 0)

        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Length(), 400)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Length(), 8)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1), (1.5, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.Length(), 8)

    def test_Contour_Area(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.Count()))

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

    def test_Contour_Centroid(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.Count()))

        startTime = time.perf_counter()
        x, y = contr.Centroid()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertAlmostEqual(x, 1000, delta=0.001)
        self.assertAlmostEqual(y, 1000, delta=0.001)

        contr = mvlab.Contour_GenEmpty()
        x, y = contr.Centroid()
        self.assertAlmostEqual(x, 0)
        self.assertAlmostEqual(y, 0)

        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.Centroid()
        self.assertAlmostEqual(x, 150)
        self.assertAlmostEqual(y, 150)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.Centroid()
        self.assertAlmostEqual(x, 2)
        self.assertAlmostEqual(y, 2)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1), (1.5, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y = contr.Centroid()
        self.assertAlmostEqual(x, 2)
        self.assertAlmostEqual(y, 2)

    def test_Contour_Point(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.Count()))

        startTime = time.perf_counter()
        b = contr.TestPoint((1000, 1000))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertTrue(b)
        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertTrue(contr.TestPoint((150, 150)))
        self.assertFalse(contr.TestPoint((150, 50)))
        self.assertFalse(contr.TestPoint((150, 250)))
        self.assertFalse(contr.TestPoint((50, 150)))
        self.assertFalse(contr.TestPoint((250, 150)))

        points = [(15, 190), (230, 190), (230, 45), (50, 45), (50, 100), (185, 100), (185, 145), (115, 145), (115, 10), (15, 10)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertTrue(contr.TestPoint((95, 115)))
        self.assertFalse(contr.TestPoint((150, 120)))
        self.assertFalse(contr.TestPoint((80, 70)))

        rgn = mvlab.Region_GenPolygon(points)
        extradata.SaveRegion(self.id(), rgn)

if __name__ == '__main__':
    unittest.main()
