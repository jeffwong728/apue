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

    @unittest.skip("")
    def test_Scrach_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.perfData.setdefault(self.id(), "{0:.3f}ms".format((endTime-startTime)*1000))

        self.assertEqual(r, 0, "Contour 'scrach.png' error")

    @unittest.skip("")
    def test_Mista_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.perfData.setdefault(self.id(), "{0:.3f}ms".format((endTime-startTime)*1000))

        self.assertEqual(r, 0, "Contour 'mista.png' error")

    @unittest.skip("")
    def test_Digits_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 151, 255)
        startTime = time.perf_counter()
        r, outer = rgn.GetContour()
        endTime = time.perf_counter()
        extradata.perfData.setdefault(self.id(), "{0:.3f}ms".format((endTime-startTime)*1000))

        self.assertEqual(r, 0, "Contour 'digits.png' error")

if __name__ == '__main__':
    unittest.main()
