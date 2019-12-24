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

class TestContourConvexHullMelkman(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        mvlab.SetGlobalOption('convex_hull_method', 'Melkman')
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Simple_Convex(self):
        contr = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (150, 150), (200, 200), (200, 100)])
        hull = contr.GetConvex()
        self.assertEqual(4, hull.CountPoints())
        extradata.SaveContours(self.id(), [hull, contr])

    def test_Simple_CCW(self):
        contr = mvlab.Contour_GenPolygon([(150, 150), (200, 200), (200, 150), (200, 100), (100, 100), (100, 100), (100, 200)])
        hull = contr.GetConvex()
        self.assertEqual(4, hull.CountPoints())
        extradata.SaveContours(self.id(), [hull, contr])

    def test_Simple_CW(self):
        contr = mvlab.Contour_GenPolygon([(100, 200), (100, 100), (100, 100), (200, 100), (200, 150), (200, 200), (150, 150)])
        hull = contr.GetConvex()
        self.assertEqual(4, hull.CountPoints())
        extradata.SaveContours(self.id(), [hull, contr])

    def test_Tangency_CCW(self):
        contr = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (200, 200), (200, 300), (300, 300), (300, 200), (200, 200), (200, 100)])
        hull = contr.GetConvex()
        extradata.SaveContours(self.id(), [hull, contr])
        self.assertEqual(6, hull.CountPoints())
        self.assertAlmostEqual(100*100*3, hull.Area())

    def test_Tangency_CW(self):
        contr = mvlab.Contour_GenPolygon([(200, 100), (200, 200), (300, 200), (300, 300), (200, 300), (200, 200), (100, 200), (100, 100)])
        hull = contr.GetConvex()
        extradata.SaveContours(self.id(), [hull, contr])
        self.assertEqual(6, hull.CountPoints())
        self.assertAlmostEqual(100*100*3, hull.Area())

    def test_Region_Contour_Convex(self):
        image = numpy.zeros((480, 640, 1), numpy.uint8)
        image[100:200, 100:200] = 255
        image[200:300, 200:300] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        contr = rgn.GetContour()
        hull = contr.GetConvex()
        extradata.SaveContours(self.id(), [hull, contr])
        self.assertEqual(6, hull.CountPoints())
        self.assertAlmostEqual(100*100*3, hull.Area())

    def test_Performance_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        c = rgn.GetContour()
        startTime = time.perf_counter()
        ch = c.GetConvex()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [c, ch])

    def test_Performance_Digits(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 50, 255)

        c = rgn.GetContour()
        startTime = time.perf_counter()
        ch = c.GetConvex()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [c, ch])

if __name__ == '__main__':
    unittest.main()
