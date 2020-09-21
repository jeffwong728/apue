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

class TestContourMoment(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Circle_EllipticAxis(self):
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 5, 'negative')
        startTime = time.perf_counter()
        e = contr.EllipticAxis()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])
        self.assertAlmostEqual(e[0], 500, delta=0.1)
        self.assertAlmostEqual(e[1], 500, delta=0.1)
        self.assertAlmostEqual(e[2], 0, delta=0.1)

    def test_Ellipse_EllipticAxis(self):
        contr = mvlab.Contour_GenEllipse((1000, 1000), (500, 300), 5, 'negative')
        startTime = time.perf_counter()
        e = contr.EllipticAxis()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])
        self.assertAlmostEqual(e[0], 500, delta=0.1)
        self.assertAlmostEqual(e[1], 300, delta=0.1)
        self.assertAlmostEqual(e[2], 0, delta=0.1)

    def test_Ellipse90_EllipticAxis(self):
        contr = mvlab.Contour_GenEllipse((1000, 1000), (300, 500), 5, 'negative')
        startTime = time.perf_counter()
        e = contr.EllipticAxis()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])
        self.assertAlmostEqual(e[0], 500, delta=0.1)
        self.assertAlmostEqual(e[1], 300, delta=0.1)
        self.assertAlmostEqual(e[2], 90, delta=0.1)

    def test_RotatedEllipse_EllipticAxis(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 5, 'positive')
        startTime = time.perf_counter()
        e = contr.EllipticAxis()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])
        self.assertAlmostEqual(e[0], 500, delta=0.1)
        self.assertAlmostEqual(e[1], 300, delta=0.1)
        self.assertAlmostEqual(e[2], 30, delta=0.1)

    def test_RotatedRectangle_EllipticAxis(self):
        contr = mvlab.Contour_GenRotatedRectangle(((1000, 1000), (500, 300), 30))
        startTime = time.perf_counter()
        e = contr.EllipticAxis()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr, mvlab.Contour_GenRotatedEllipse((1000, 1000), (e[0], e[1]), e[2], 5, 'positive')])
        self.assertAlmostEqual(e[0], 288.67524294716094, delta=0.1)
        self.assertAlmostEqual(e[1], 173.20556894940879, delta=0.1)
        self.assertAlmostEqual(e[2], 30, delta=0.1)

if __name__ == '__main__':
    unittest.main()
