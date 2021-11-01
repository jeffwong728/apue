import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionUnion(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_SameRegion_Union(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        rgn1 = mvlab.Threshold(image1, 150, 255)

        uRgn = rgn1.Union2(rgn1)
        self.assertEqual(uRgn.CountRuns(), rgn1.CountRuns())
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area())

    def test_AdjacentHBox_Union(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        rgn1 = mvlab.Threshold(image1, 150, 255)

        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 20:30] = 255
        rgn2 = mvlab.Threshold(image2, 150, 255)

        uRgn = rgn1.Union2(rgn2)
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area() + rgn2.Area())
        self.assertEqual(uRgn.CountRuns(), 10)

    def test_3HBox_Union(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        image1[10:20, 30:40] = 255
        rgn1 = mvlab.Threshold(image1, 150, 255)

        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 20:30] = 255
        rgn2 = mvlab.Threshold(image2, 150, 255)

        uRgn = rgn1.Union2(rgn2)
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area() + rgn2.Area())
        self.assertEqual(uRgn.CountRuns(), 10)

    def test_3HBox_Overlap_Union(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        image1[10:20, 30:40] = 255
        rgn1 = mvlab.Threshold(image1, 150, 255)

        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 15:35] = 255
        rgn2 = mvlab.Threshold(image2, 150, 255)

        uRgn = rgn1.Union2(rgn2)
        self.assertAlmostEqual(uRgn.Area(), 300)
        self.assertEqual(uRgn.CountRuns(), 10)

    def test_2VBox_Union(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[0:20, 10:20] = 255
        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:30, 5:25] = 255
        rgn1 = mvlab.Threshold(image1, 150, 255)
        rgn2 = mvlab.Threshold(image2, 150, 255)
        interRgn = rgn1.Union2(rgn2)
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

        self.assertEqual(interRgn.CountRows(), 30)

    def test_2Circle_Overlap_Union(self):
        rgn1 = mvlab.Region_GenCircle((1250, 1250), 750)
        rgn2 = mvlab.Region_GenCircle((2000, 1250), 700)

        startTime = time.perf_counter()
        uRgn = rgn1.Union2(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), uRgn)
        interRgn = rgn1.Intersection(rgn2)
        self.assertAlmostEqual(rgn1.Area()+rgn2.Area()-interRgn.Area(), uRgn.Area())

    def test_Mista_Box_Union(self):
        image1 = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        startTime = time.perf_counter()
        interRgn = rgn1.Union2(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

    def test_Mista_Circle_Union(self):
        image1 = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        interRgn = rgn1.Union2(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

    def test_Mista_RotatedEllipse_Union(self):
        image1 = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRotatedEllipse((1250, 1250), (750, 500), 30)

        startTime = time.perf_counter()
        interRgn = rgn1.Union2(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), interRgn, image1.shape)

if __name__ == '__main__':
    unittest.main()
