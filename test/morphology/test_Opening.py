import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestOpening(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if sys.platform == 'win32':
            logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                        filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                        filemode='a')

    def test_Opening_Bumps(self):
        se = mvlab.Region_GenStructuringElement('circle', 11)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'bumps.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 0, 50)

        startTime = time.perf_counter()
        ergn = rgn.Opening(se)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(ergn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), ergn)

    def test_Opening_Circle_Mista(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 128, 255)

        startTime = time.perf_counter()
        ergn = rgn.Opening(se)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(ergn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), ergn)

    def test_Opening_Square_Mista(self):
        se = mvlab.Region_GenStructuringElement('square', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 128, 255)

        startTime = time.perf_counter()
        ergn = rgn.Opening(se)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(ergn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), ergn)

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
