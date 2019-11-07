import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging

class TestContour(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Scrach_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outers = rgn.OuterContours()
        endTime = time.perf_counter()
        logging.info("Contour 'scrach.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(outers), 94, "Contour 'scrach.png' error")

    def test_Mista_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outers = rgn.OuterContours()
        endTime = time.perf_counter()
        logging.info("Contour 'mista.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(outers), 941, "Contour 'mista.png' error")

    def test_Digits_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 151, 255)
        startTime = time.perf_counter()
        r, outers = rgn.OuterContours()
        endTime = time.perf_counter()
        logging.info("Contour 'digits.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(outers), 5584, "Contour 'digits.png' error")

if __name__ == '__main__':
    unittest.main()
