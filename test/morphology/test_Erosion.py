import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestErosion(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if sys.platform == 'win32':
            logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                        filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                        filemode='a')

    def test_Bumps(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'bumps.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        rgn = mvlab.Threshold(blue, 0, 50)

        startTime = time.perf_counter()
        ergn = rgn.Erosion(se)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(ergn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), ergn)

    def test_RLEErosion(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'bumps.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        rgn = mvlab.Threshold(blue, 0, 50)

        opts = mvlab.Dict_GenEmpty()
        opts.SetString("Method", "RLEErosion")

        startTime = time.perf_counter()
        ergn = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(ergn)

        e2cutrgn = rgn.Erosion(se)
        self.assertTrue(e2cutrgn.TestEqual(ergn))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), ergn)

    def test_Mista(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        rgn = mvlab.Threshold(blue, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "erode1")
        startTime = time.perf_counter()
        rgn1 = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn1)
        print('erode1: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "erode2")
        startTime = time.perf_counter()
        rgn2 = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn2)
        print('erode2: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "erode2cut")
        startTime = time.perf_counter()
        rgn2cut = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn2cut)
        print('erode2cut: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEErosion")
        startTime = time.perf_counter()
        rlergn = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rlergn)
        print('RLEErosion: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "erode3")
        startTime = time.perf_counter()
        rgn3 = rgn.Erosion(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn3)
        print('erode3: {0:.3f}ms'.format((endTime-startTime)*1000))

        self.assertTrue(rgn1.TestEqual(rgn2))
        self.assertTrue(rgn2.TestEqual(rgn2cut))
        self.assertTrue(rgn2cut.TestEqual(rgn3))
        self.assertTrue(rgn3.TestEqual(rlergn))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rlergn)

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

    def dumpRegion(self, rgn):
        if not rgn:
            return
        runs = rgn.GetRuns()
        for run in runs:
            print(run)
