import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionDifference(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_2VBox_Difference(self):
        image1 = numpy.zeros((48, 64, 1), numpy.uint8)
        image1[0:20, 10:20] = 255
        image2 = numpy.zeros((48, 64, 1), numpy.uint8)
        image2[10:30, 5:25] = 255
        r, rgn1 = mvlab.Threshold(image1, 150, 255)
        r, rgn2 = mvlab.Threshold(image2, 150, 255)
        diffRgn = rgn1.Difference(rgn2)
        extradata.SaveRegion(self.id(), diffRgn, image1.shape)

        self.assertEqual(diffRgn.CountRows(), 10)

    def test_SameRegion_Difference(self):
        rgn1 = mvlab.Region_CreateCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn1)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn)
        self.assertAlmostEqual(0.0, diffRgn.Area())

    def test_2Circle_Include_Difference(self):
        rgn1 = mvlab.Region_CreateCircle((1250, 1250), 750)
        rgn2 = mvlab.Region_CreateCircle((1250, 1250), 700)

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn)
        self.assertAlmostEqual(rgn1.Area()-rgn2.Area(), diffRgn.Area())

    def test_2Circle_Overlap_Difference(self):
        rgn1 = mvlab.Region_CreateCircle((1250, 1250), 750)
        rgn2 = mvlab.Region_CreateCircle((2000, 1250), 700)

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn)

        interRgn = rgn1.Intersection(rgn2)
        self.assertAlmostEqual(rgn1.Area()-interRgn.Area(), diffRgn.Area())

    def test_Mista_Box_Difference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_CreateRectangle((1000, 1000, 1000, 1000))

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn, image1.shape)

    def test_Mista_Circle_Difference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_CreateCircle((1250, 1250), 750)

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn, image1.shape)

    def test_Mista_RotatedEllipse_Difference(self):
        image1 = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image1)
        r, rgn1 = mvlab.Threshold(blue, 150, 255)
        rgn2 = mvlab.Region_CreateRotatedEllipse((1250, 1250), (750, 500), 30)

        startTime = time.perf_counter()
        diffRgn = rgn1.Difference(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), diffRgn, image1.shape)

if __name__ == '__main__':
    unittest.main()
