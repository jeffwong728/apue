import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging

class TestRegionComplement(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Box_Complement(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:20, 10:20] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        compRgn = rgn.Complement((0, 0, 0, 0))

        self.assertEqual(compRgn.Count(), 22)
        self.assertEqual(compRgn.CountRows(), 12)
        self.assertAlmostEqual(compRgn.Area(), 44)

    def test_2VBox_Complement(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[0:10, 10:20] = 255
        image[20:30, 10:20] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        compRgn = rgn.Complement((0, 0, 0, 0))

        self.assertEqual(compRgn.Count(), 52)
        self.assertEqual(compRgn.CountRows(), 32)
        self.assertAlmostEqual(compRgn.Area(), 184)

    def test_2HBox_Complement(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[0:10, 10:20] = 255
        image[0:10, 30:40] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        compRgn = rgn.Complement((0, 0, 0, 0))

        self.assertEqual(compRgn.Count(), 32)
        self.assertEqual(compRgn.CountRows(), 12)
        self.assertAlmostEqual(compRgn.Area(), 184)

    def test_2Hole_Complement(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[0:10, 10:20] = 255
        image[3:7, 13:17] = 0
        image[0:10, 30:40] = 255
        image[3:7, 33:37] = 0
        r, rgn = mvlab.Threshold(image, 150, 255)
        compRgn = rgn.Complement((0, 0, 0, 0))

        r, rgns = compRgn.Connect(8)
        self.assertEqual(len(rgns), 3)

    def test_Scrach_Complement(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        compRgn = rgn.Complement((0, 0, 0, 0))
        endTime = time.perf_counter()
        logging.info("Complement 'scrach.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(compRgn.Count(), 4348)

    def test_Mista_Complement(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        compRgn = rgn.Complement((0, 0, 0, 0))
        endTime = time.perf_counter()
        logging.info("Complement 'mista.png' spent {0:f}ms".format((endTime-startTime)*1000))

        r, rgns = compRgn.Connect(8)
        self.assertEqual(len(rgns), 1072)

    def test_Digits_Complement(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        compRgn = rgn.Complement((0, 0, 0, 0))
        endTime = time.perf_counter()
        logging.info("Complement 'digits.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(compRgn.Count(), 90084, 'Digits component number error')

if __name__ == '__main__':
    unittest.main()
