import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestThreshold(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Threshold_Single_Pixel(self):
        image = numpy.zeros((1, 1, 1), numpy.uint8)
        image[0, 0] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        self.assertEqual(r, 0, "Threshold single pixel image error")
        self.assertEqual(rgn.Count(), 1)
        self.assertEqual(rgn.CountRows(), 1)
        self.assertAlmostEqual(rgn.Area(), 1.0)
        self.assertEqual(rgn.BoundingBox(), (0, 0, 1, 0))

    def test_Threshold_Single_Row(self):
        for startCol in range(0, 65):
            for lastCol in range(startCol+1, 66):
                image = numpy.zeros((1, 65, 1), numpy.uint8)
                image[0, startCol:lastCol] = 255
                r, rgn = mvlab.Threshold(image, 150, 255)
                self.assertEqual(r, 0, "Threshold single pixel image error")
                self.assertEqual(rgn.Count(), 1)
                self.assertEqual(rgn.CountRows(), 1)
                self.assertAlmostEqual(rgn.Area(), lastCol-startCol)
                self.assertEqual(rgn.BoundingBox(), (startCol, 0, lastCol-startCol, 0))

    def test_Scrach_Threshold(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)

        startTime = time.perf_counter()
        r, rgn = mvlab.Threshold(blue, 150, 255)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn, image.shape)

        self.assertEqual(r, 0, "Threshold 'scrach.png' error")

    def test_Mista_Threshold(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        startTime = time.perf_counter()
        r, rgn = mvlab.Threshold(blue, 150, 255)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn, image.shape)

        self.assertEqual(r, 0, "Threshold 'mista.png' error")
        self.assertEqual(rgn.Count(), 234794)

    def test_Digits_Threshold(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        startTime = time.perf_counter()
        r, rgn = mvlab.Threshold(blue, 150, 255)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn, image.shape)

        self.assertEqual(r, 0, "Threshold 'digits.png' error")

    def test_PCB_Layout_Threshold(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'models', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)

        startTime = time.perf_counter()
        r, rgn = mvlab.Threshold(image, 0, 50)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn, image.shape)

        self.assertEqual(r, 0)

if __name__ == '__main__':
    unittest.main()
