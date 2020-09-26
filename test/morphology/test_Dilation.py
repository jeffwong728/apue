import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestDilation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Bumps(self):
        se = mvlab.Region_GenStructuringElement('circle', 11)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'bumps.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 0, 50)

        startTime = time.perf_counter()
        drgn = rgn.Dilation(se)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(drgn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), drgn)

    def test_RLEDilation(self):
        se = mvlab.Region_GenStructuringElement('circle', 11)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'bumps.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 0, 50)

        opts = mvlab.Dict_GenEmpty()
        opts.SetString("Method", "RLEDilation")

        startTime = time.perf_counter()
        drgn = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(drgn)
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), drgn)

        drgn1 = rgn.Dilation(se)
        self.assertTrue(drgn1.TestEqual(drgn))

    def test_Mista_Circle(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "dilate")
        startTime = time.perf_counter()
        drgn = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(drgn)
        print('dilate: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "dilatecut")
        startTime = time.perf_counter()
        dcutrgn = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(dcutrgn)
        print('dilatecut: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEComp")
        startTime = time.perf_counter()
        comprgn = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(comprgn)
        print('comprgn: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEDilation")
        startTime = time.perf_counter()
        rlergn = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rlergn)
        print('RLEDilation: {0:.3f}ms'.format((endTime-startTime)*1000))

        self.assertTrue(drgn.TestEqual(dcutrgn))
        self.assertTrue(dcutrgn.TestEqual(comprgn))
        self.assertTrue(comprgn.TestEqual(rlergn))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), comprgn)

    def test_Mista_Square(self):
        se = mvlab.Region_GenStructuringElement('square', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "dilate")
        startTime = time.perf_counter()
        rgn1 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn1)
        print('dilate: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEComp")
        startTime = time.perf_counter()
        rgn2 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn2)
        print('comprgn: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "dilatecut")
        startTime = time.perf_counter()
        rgn3 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn3)
        print('dilatecut: {0:.3f}ms'.format((endTime-startTime)*1000))

        self.assertTrue(rgn1.TestEqual(rgn2))
        self.assertTrue(rgn2.TestEqual(rgn3))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn1)

    def test_Mista_Outer_Boundary(self):
        se = mvlab.Region_GenStructuringElement('square', 1)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "RLEComp")
        startTime = time.perf_counter()
        drgn = rgn.Dilation(se, opts)
        drgn = drgn.Difference(rgn)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(drgn)

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), drgn)

    def test_Mista_Inner_Boundary(self):
        se = mvlab.Region_GenStructuringElement('square', 1)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "RLEErosion")
        startTime = time.perf_counter()
        drgn = rgn.Erosion(se, opts)
        drgn = rgn.Difference(drgn)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(drgn)

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), drgn)

    def test_Digits_Circle(self):
        se = mvlab.Region_GenStructuringElement('circle', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'digits.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "dilate")
        startTime = time.perf_counter()
        rgn1 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn1)
        print('dilate: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEDilation")
        startTime = time.perf_counter()
        rgn2 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn2)
        print('RLEDilation: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEComp")
        startTime = time.perf_counter()
        rgn3 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn3)
        print('RLEComp: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "dilatecut")
        startTime = time.perf_counter()
        rgn4 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn4)
        print('dilatecut: {0:.3f}ms'.format((endTime-startTime)*1000))

        self.assertTrue(rgn1.TestEqual(rgn2))
        self.assertTrue(rgn2.TestEqual(rgn3))
        self.assertTrue(rgn3.TestEqual(rgn4))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn3)

    def test_Digits_Square(self):
        se = mvlab.Region_GenStructuringElement('square', 5)
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'digits.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 150, 255)
        opts = mvlab.Dict_GenEmpty()

        opts.SetString("Method", "dilate")
        startTime = time.perf_counter()
        rgn1 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn1)
        print('dilate: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEDilation")
        startTime = time.perf_counter()
        rgn2 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn2)
        print('RLEDilation: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "RLEComp")
        startTime = time.perf_counter()
        rgn3 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn3)
        print('RLEComp: {0:.3f}ms<br />'.format((endTime-startTime)*1000))

        opts.SetString("Method", "dilatecut")
        startTime = time.perf_counter()
        rgn4 = rgn.Dilation(se, opts)
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(rgn4)
        print('dilatecut: {0:.3f}ms'.format((endTime-startTime)*1000))

        self.assertTrue(rgn1.TestEqual(rgn2))
        self.assertTrue(rgn2.TestEqual(rgn3))
        self.assertTrue(rgn3.TestEqual(rgn4))

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegion(self.id(), rgn2)

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