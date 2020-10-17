import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionIntersection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_2VBox_Intersection(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[0:20, 10:20] = 255
        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:30, 5:25] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)
        r, rgn2 = mvlab.Threshold(image2, 150, 255)
        interRgn = rgn1.Intersection(rgn2)
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

        self.assertEqual(interRgn.CountRows(), 10)

    def test_Mista_Box_Intersection(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        startTime = time.perf_counter()
        interRgn = rgn1.Intersection(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

    def test_Mista_Circle_Intersection(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        interRgn = rgn1.Intersection(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

    def test_Mista_RotatedEllipse_Intersection(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRotatedEllipse((1250, 1250), (750, 500), 30)

        startTime = time.perf_counter()
        interRgn = rgn1.Intersection(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

if __name__ == '__main__':
    unittest.main()
