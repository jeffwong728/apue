import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging

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
        rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        logging.info("Connect 'scrach.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(rgns.Count(), 94, 'Scrach component number error')

    def test_Scrach_Connection2(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect2(8)
        endTime = time.perf_counter()
        logging.info("Connect2 'scrach.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(rgns), 94, 'Scrach component number error')

    def test_Mista_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        logging.info("Connect 'mista.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(rgns.Count(), 941, 'Mista component number error')

    def test_Mista_Connection2(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect2(8)
        endTime = time.perf_counter()
        logging.info("Connect2 'mista.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(rgns), 941, 'Mista component number error')

    def test_Digits_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        rgns = rgn.Connect(8)
        endTime = time.perf_counter()
        logging.info("Connect 'digits.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(rgns.Count(), 5584, 'Digits component number error')

    def test_Digits_Connection2(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        r, rgns = rgn.Connect2(8)
        endTime = time.perf_counter()
        logging.info("Connect2 'digits.png' spent {0:f}ms".format((endTime-startTime)*1000))

        self.assertEqual(len(rgns), 5584, 'Digits component number error')

if __name__ == '__main__':
    unittest.main()
