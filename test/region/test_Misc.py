import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestMisc(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if sys.platform == 'win32':
            logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                        filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                        filemode='a')

    def test_CountResistor(self):
        se = mvlab.Region_GenStructuringElement('circle', 11)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mix-cs8svf-pcb.jpg'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        rgn = mvlab.Threshold(blue, 90, 255)

        startTime = time.perf_counter()
        ergn = rgn.Opening(se)
        height, width, channels = image.shape
        compRgn = ergn.Complement((-1, -1, width+3, height+3))
        rrgn = rgn.Intersection(compRgn)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rrgn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rrgn)

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
