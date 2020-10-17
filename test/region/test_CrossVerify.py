import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import random
import logging
import extradata

class TestRegionCrossVerify(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_Mista_Box_Diff_Plus_Inter(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        dRgn = rgn1.Difference(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        uRgn = dRgn.Union2(iRgn)

        self.assertEqual(uRgn.CountRuns(), rgn1.CountRuns())
        self.assertEqual(uRgn.CountRows(), rgn1.CountRows())
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area())

    def test_Mista_Box_Diff_Plus_Diff(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        dRgn1 = rgn1.Difference(rgn2)
        dRgn2 = rgn2.Difference(rgn1)
        uRgn = dRgn1.Union2(dRgn2)
        sRgn = rgn1.SymmDifference(rgn2)

        self.verifyRegionIntegrity(dRgn1)
        self.verifyRegionIntegrity(dRgn2)
        self.verifyRegionIntegrity(uRgn)
        self.verifyRegionIntegrity(sRgn)

        self.assertEqual(sRgn.CountRuns(), uRgn.CountRuns())
        self.assertEqual(sRgn.CountRows(), uRgn.CountRows())
        self.assertAlmostEqual(sRgn.Area(), uRgn.Area())

    def test_Mista_Box_Union_Minus_Inter(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRectangle((1000, 1000, 1000, 1000))

        dRgn1 = rgn1.Difference(rgn2)
        dRgn2 = rgn2.Difference(rgn1)
        sRgn1  = dRgn1.Union2(dRgn2)

        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        sRgn2 = uRgn.Difference(iRgn)

        self.verifyRegionIntegrity(uRgn)
        self.verifyRegionIntegrity(iRgn)
        self.verifyRegionIntegrity(sRgn2)
        self.assertEqual(sRgn1.CountRuns(), sRgn2.CountRuns())
        self.assertEqual(sRgn1.CountRows(), sRgn2.CountRows())
        self.assertAlmostEqual(sRgn1.Area(), sRgn2.Area())

    def test_Mista_Circle_Diff_Plus_Inter(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenCircle((1250, 1250), 750)

        dRgn = rgn1.Difference(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        uRgn = dRgn.Union2(iRgn)

        self.assertEqual(uRgn.CountRuns(), rgn1.CountRuns())
        self.assertEqual(uRgn.CountRows(), rgn1.CountRows())
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area())

    def test_Mista_RotatedEllipse_Diff_Plus_Inter(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_GenRotatedEllipse((1250, 1250), (750, 500), 30)

        dRgn = rgn1.Difference(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        uRgn = dRgn.Union2(iRgn)

        self.assertEqual(uRgn.CountRuns(), rgn1.CountRuns())
        self.assertEqual(uRgn.CountRows(), rgn1.CountRows())
        self.assertAlmostEqual(uRgn.Area(), rgn1.Area())

    def test_2Circle_Overlap_Diff_Plus_Diff(self):
        rgn1 = mvlab.Region_GenCircle((1250, 1250), 750)
        rgn2 = mvlab.Region_GenCircle((2000, 1250), 700)

        dRgn1 = rgn1.Difference(rgn2)
        dRgn2 = rgn2.Difference(rgn1)
        sRgn = rgn1.SymmDifference(rgn2)
        uRgn = dRgn1.Union2(dRgn2)

        self.assertEqual(uRgn.CountRuns(), sRgn.CountRuns())
        self.assertEqual(uRgn.CountRows(), sRgn.CountRows())
        self.assertAlmostEqual(uRgn.Area(), sRgn.Area())

    def test_Random_Rectangles_Union(self):
        random.seed()
        image1 = numpy.zeros((480, 640, 1), numpy.uint8)
        for i in range(0, 100):
            x = int(random.uniform(0, 640))
            y = int(random.uniform(0, 640))
            w = int(random.uniform(0, 80))
            h = int(random.uniform(0, 60))
            cv2.rectangle(image1, (x, y), (x+w, y+h), (255, 255, 255), -1)

        image2 = numpy.zeros((480, 640, 1), numpy.uint8)
        for i in range(0, 50):
            x = int(random.uniform(0, 640))
            y = int(random.uniform(0, 640))
            w = int(random.uniform(0, 320))
            h = int(random.uniform(0, 30))
            cv2.rectangle(image2, (x, y), (x+w, y+h), (255, 255, 255), -1)

        r, rgn1 = mvlab.Threshold(image1, 150, 255)
        r, rgn2 = mvlab.Threshold(image2, 150, 255)
        sRgn1 = rgn1.SymmDifference(rgn2)
        uRgn = rgn1.Union2(rgn2)
        iRgn = rgn1.Intersection(rgn2)
        sRgn2 = uRgn.Difference(iRgn)

        self.verifyRegionIntegrity(uRgn)
        self.verifyRegionIntegrity(sRgn1)
        self.assertEqual(sRgn1.CountRuns(), sRgn2.CountRuns())
        self.assertEqual(sRgn1.CountRows(), sRgn2.CountRows())
        self.assertAlmostEqual(sRgn1.Area(), sRgn2.Area())

        extradata.SaveRegion(self.id(), sRgn1, image1.shape)

    def verifyRegionIntegrity (self, rgn):
        if not rgn:
            return

        runs = rgn.GetRuns()
        if len(runs):
            self.assertLess(runs[0][0], runs[0][2])
            for i in range(1, len(runs)):
                self.assertLessEqual(runs[i-1][1], runs[i][1])
                self.assertLess(runs[i][0], runs[i][2])
                if runs[i-1][1] == runs[i][1]:
                    self.assertLess(runs[i-1][2], runs[i][0])

if __name__ == '__main__':
    unittest.main()
