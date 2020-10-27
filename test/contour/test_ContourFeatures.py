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
        pass

    def test_Contour_BoundingBox(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.CountPoints()))

        startTime = time.perf_counter()
        x, y, w, h = contr.GetBoundingBox()[0]
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)

        self.assertEqual(x, 500)
        self.assertEqual(y, 500)
        self.assertEqual(w, 1000)
        self.assertEqual(h, 1000)

        contr = mvlab.Contour_GenEmpty()
        x, y, w, h = contr.GetBoundingBox()[0]
        self.assertEqual(x, 0)
        self.assertEqual(y, 0)
        self.assertEqual(w, 0)
        self.assertEqual(h, 0)

        points = [(100, 100), (50, 200), (150, 150), (250, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        x, y, w, h = contr.GetBoundingBox()[0]
        self.assertEqual(x, 50)
        self.assertEqual(y, 100)
        self.assertEqual(w, 200)
        self.assertEqual(h, 100)

    def test_Contour_Length(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.CountPoints()))
        startTime = time.perf_counter()
        l = contr.GetLength()[0]
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertAlmostEqual(l, 2*math.pi*500, delta=0.001)

        contr = mvlab.Contour_GenEmpty()
        self.assertAlmostEqual(contr.GetLength()[0], 0)

        points = [(100, 100), (100, 200), (200, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.GetLength()[0], 400)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.GetLength()[0], 8)

        points = [(1, 1), (1, 2), (1, 3), (2, 3), (3, 3), (3, 2), (3, 1), (2, 1), (1.5, 1)]
        contr = mvlab.Contour_GenPolygon(points)
        self.assertAlmostEqual(contr.GetLength()[0], 8)

    def test_Contour_Area(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.CountPoints()))

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
        print('Contour Number of Points: {0:d}'.format(contr.CountPoints()))

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

    def test_Contour_Point(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        print('Contour Number of Points: {0:d}'.format(contr.CountPoints()))

        startTime = time.perf_counter()
        b = contr.TestPoint((1000, 1000))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertTrue(b)

        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'positive')
        self.assertTrue(contr.TestPoint((1000, 1000)))
        self.assertFalse(contr.TestPoint((2000, 2000)))

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

    def test_Contour_Convex(self):
        contr1 = mvlab.Contour_GenCircle((50, 50), 20, 1, 'negative')
        self.assertTrue(contr1.TestConvex())

        contr2 = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (200, 200), (200, 100)])
        self.assertTrue(contr2.TestConvex())

        contr3 = mvlab.Contour_GenPolygon([(15, 190), (230, 190), (230, 45), (50, 45), (50, 100), (185, 100), (185, 145), (115, 145), (115, 10), (15, 10)])
        self.assertFalse(contr3.TestConvex())
        self.assertTrue(contr3.GetConvex().TestConvex())

        contr4 = mvlab.Contour_GenPolygon([(200, 100), (100, 200), (200, 150), (300, 200)])
        self.assertFalse(contr4.TestConvex())
        self.assertTrue(contr4.GetConvex().TestConvex())

        contr5 = mvlab.Contour_GenPolygon([(100, 100), (100, 150), (100, 200), (200, 200), (200, 100)])
        self.assertFalse(contr5.TestConvex())
        self.assertTrue(contr5.GetConvex().TestConvex())

        contr6 = mvlab.Contour_GenPolygon([(100, 100), (50, 150), (100, 200), (200, 200), (200, 100)])
        self.assertTrue(contr6.TestConvex())

        contr7 = mvlab.Contour_GenPolygon([(100, 100), (50, 150), (50, 150), (100, 200), (200, 200), (200, 100)])
        self.assertFalse(contr7.TestConvex())
        self.assertTrue(contr7.GetConvex().TestConvex())

        contr8 = mvlab.Contour_GenPolygon([(100, 100), (50, 150), (100, 200), (200, 200), (200, 100), (100, 100)])
        self.assertTrue(contr8.TestConvex())
        self.assertTrue(contr8.GetConvex().TestConvex())

        extradata.SaveContours(self.id(), [contr1, contr2, contr3, contr4, contr5, contr6, contr7, contr8])

    def test_Contour_Circularity(self):
        c = mvlab.Contour_GenCircle((1000, 1000), 500, 1, 'negative')
        self.assertAlmostEqual(c.Circularity(), 1.0, delta=0.001)

        e = mvlab.Contour_GenEllipse((1000, 1000), (500, 300), 5, 'negative')
        self.assertAlmostEqual(e.Circularity(), 0.6, delta=0.001)

        e = mvlab.Contour_GenEllipse((1000, 1000), (500, 100), 5, 'negative')
        self.assertAlmostEqual(e.Circularity(), 0.2, delta=0.001)

        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'digits.png'))
        blue, green, red = cv2.split(image)
        rgn = mvlab.Threshold(blue, 151, 255)
        rgns = rgn.Connect()

        roundDigits = []
        for i in range(0, rgns.Count()):
            rgn = rgns.SelectObj(i)
            c = rgn.GetContour()
            if c.Circularity() > 0.6 and c.Area() > 10:
                roundDigits.append(c)
        extradata.SaveContours(self.id(), roundDigits)

    def test_Contour_SelfIntersection(self):
        contr1 = mvlab.Contour_GenPolygon([(100, 100), (200, 200), (100, 200), (200, 100)])
        self.assertTrue(contr1.TestSelfIntersection('false'))

        contr2 = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (200, 100), (200, 200)])
        self.assertTrue(contr2.TestSelfIntersection('false'))

if __name__ == '__main__':
    unittest.main()
