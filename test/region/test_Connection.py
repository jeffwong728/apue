import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionConnection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Scrach_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(len(rgns), 94, 'Scrach component number error')

    def test_Mista_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(len(rgns), 941, 'Mista component number error')

    def test_Digits_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(len(rgns), 5584, 'Digits component number error')

if __name__ == '__main__':
    unittest.main()
