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

class TestRegionFeatures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Digits_ConvexHull(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 50, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour()
        mvlab.SetGlobalOption('convex_hull_method', 'Melkman')
        startTime = time.perf_counter()
        ch = c.GetConvex()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [c, ch])

    def test_Mista_ConvexHull(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour()
        mvlab.SetGlobalOption('convex_hull_method', 'Melkman')
        startTime = time.perf_counter()
        ch = c.GetConvex()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [c, ch])

    def test_Digits_MiniCircle(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 50, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour()
        startTime = time.perf_counter()
        ch = c.GetSmallestCircle()
        endTime = time.perf_counter()

        conts = [c]
        for mc in ch:
            conts.append(mvlab.Contour_GenCircle((mc[0], mc[1]), mc[2], 1.5, 'negative'))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), conts)

    def test_Mista_MiniCircle(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour()
        startTime = time.perf_counter()
        ch = c.GetSmallestCircle()
        endTime = time.perf_counter()

        conts = [c]
        for mc in ch:
            conts.append(mvlab.Contour_GenCircle((mc[0], mc[1]), mc[2], 1.5, 'negative'))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), conts)

if __name__ == '__main__':
    unittest.main()
