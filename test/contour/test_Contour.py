import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestContour(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_2VBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 20:50] = 255
        image[20:25, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outer = rgn.GetContour()

        self.assertEqual(r, 0, '2Box contour number error')
        self.assertAlmostEqual(cv2.contourArea(outer.Simplify(1.0)[1]), 150.0)

    def test_2HBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 1:31] = 255
        image[10:15, 33:63] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outer = rgn.GetContour()

        self.assertEqual(r, 0, '2Box contour number error')
        self.assertAlmostEqual(cv2.contourArea(outer.Simplify(1.0)[1]), 150.0)

    def test_Simple_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:11, 1:10] = 255
        image[10:11, 15:30] = 255
        image[11:12, 5:35] = 255
        image[30:31, 1:30] = 255
        image[31:32, 5:10] = 255
        image[31:32, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outer = rgn.GetContour()

        self.assertEqual(r, 0, 'Simple contour number error')

    @unittest.skip("")
    def test_Scrach_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'scrach.png' error")

    @unittest.skip("")
    def test_Mista_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'mista.png' error")

    @unittest.skip("")
    def test_Digits_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 151, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'digits.png' error")

if __name__ == '__main__':
    unittest.main()
