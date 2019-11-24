import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata
import random

class TestRegionTransformation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Zoom_Deer(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        r, rgns = rgn.Connect()

        for rgn in rgns:
           if rgn.Area() > 140000 and rgn.Area() < 150000:
               break

        startTime = time.perf_counter()
        zrgn = rgn.Zoom((0.3, 0.3))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        rgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)

    def verifyRegionIntegrity (self, rgn):
        if not rgn:
            return

        r, runs = rgn.GetRuns()
        self.assertEqual(r, 0)
        if len(runs):
            self.assertLess(runs[0][0], runs[0][2])
            for i in range(1, len(runs)):
                self.assertLessEqual(runs[i-1][1], runs[i][1])
                self.assertLess(runs[i][0], runs[i][2])
                if runs[i-1][1] == runs[i][1]:
                    self.assertLess(runs[i-1][2], runs[i][0])

if __name__ == '__main__':
    unittest.main()