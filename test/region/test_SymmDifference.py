import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionSymmDifference(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_2VBox_SymmDifference(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[0:20, 10:20] = 255
        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:30, 5:25] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)
        r, rgn2 = mvlab.Threshold(image2, 150, 255)
        symDiffRgn = rgn1.SymmDifference(rgn2)
        extradata.SaveRegion(self.id(), symDiffRgn, image1.shape)

        self.assertEqual(symDiffRgn.CountRows(), 30)

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

    def test_HBox_SymmDifference(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 20:30] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)
        r, rgn2 = mvlab.Threshold(image2, 150, 255)
        symDiffRgn = rgn1.SymmDifference(rgn2)
        extradata.SaveRegion(self.id(), symDiffRgn, image1.shape)

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRows(), 10)
        self.assertEqual(symDiffRgn.CountRuns(), 10)
        self.assertEqual(uRgn.CountRuns(), 10)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

    def test_3HBox_SymmDifference(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        image1[10:20, 30:40] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)

        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 20:30] = 255
        r, rgn2 = mvlab.Threshold(image2, 150, 255)

        sRgn = rgn1.SymmDifference(rgn2)
        self.assertAlmostEqual(sRgn.Area(), rgn1.Area() + rgn2.Area())
        self.assertEqual(sRgn.CountRuns(), 10)

    def test_3HBox_Overlap_SymmDifference(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[10:20, 10:20] = 255
        image1[10:20, 30:40] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)

        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:20, 15:35] = 255
        r, rgn2 = mvlab.Threshold(image2, 150, 255)

        sRgn = rgn1.SymmDifference(rgn2)
        self.assertAlmostEqual(sRgn.Area(), 200)
        self.assertEqual(sRgn.CountRuns(), 30)

    def test_SameRegion_SymmDifference(self):
        rgn1 = mvlab.Region_GenCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        symDiffRgn = rgn1.SymmDifference(rgn1)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), symDiffRgn)
        self.assertAlmostEqual(0.0, symDiffRgn.Area())

        uRgn = rgn1.Union2(rgn1)
        iRgn = rgn1.Intersection(rgn1)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

    def test_Mista_Box_SymmDifference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        startTime = time.perf_counter()
        symDiffRgn = rgn1.SymmDifference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), symDiffRgn, image1.shape)

        dRgn1 = rgn1.Difference(rgn2)
        dRgn2 = rgn2.Difference(rgn1)
        uRgn = dRgn1.Union2(dRgn2)
        self.assertEqual(symDiffRgn.CountRuns(), uRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), uRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), uRgn.Area())

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

    def test_Mista_Circle_SymmDifference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        symDiffRgn = rgn1.SymmDifference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), symDiffRgn, image1.shape)

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

    def test_Mista_RotatedEllipse_SymmDifference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRotatedEllipse((1250, 1250), (750, 500), 30)

        startTime = time.perf_counter()
        symDiffRgn = rgn1.SymmDifference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), symDiffRgn, image1.shape)

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        dRgn = uRgn.Difference(iRgn)
        self.assertEqual(symDiffRgn.CountRuns(), dRgn.CountRuns())
        self.assertEqual(symDiffRgn.CountRows(), dRgn.CountRows())
        self.assertAlmostEqual(symDiffRgn.Area(), dRgn.Area())

if __name__ == '__main__':
    unittest.main()
