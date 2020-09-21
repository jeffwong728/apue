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
        rgns = rgn.Connect()

        for i in range(0, rgns.Count()):
            rgn = rgns.SelectObj(i)
            if rgn.Area() > 140000 and rgn.Area() < 150000:
                break

        startTime = time.perf_counter()
        zrgn = rgn.Zoom((0.7, 0.7))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Zoom_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        zrgn = rgn.Zoom((0.2, 0.2))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Shrink_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        zrgn = rgn.Shrink((0.2, 0.2))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Zoom_Digits(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        zrgn = rgn.Zoom((0.5, 0.5))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Shrink_Digits(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        startTime = time.perf_counter()
        zrgn = rgn.Shrink((0.5, 0.5))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Zoom_PCB_Layout(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'models', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 0, 50)

        startTime = time.perf_counter()
        zrgn = rgn.Zoom((1.2, 1.2))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Zoom_Out_PCB_Layout(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'models', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 0, 50)

        startTime = time.perf_counter()
        zrgn = rgn.Zoom((0.5, 0.5))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Shrink_PCB_Layout(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'models', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 0, 50)

        startTime = time.perf_counter()
        zrgn = rgn.Shrink((0.5, 0.5))
        endTime = time.perf_counter()
        self.verifyRegionIntegrity(zrgn)

        bbox = zrgn.BoundingBox()
        zrgn = zrgn.Move((-bbox[0]+10, -bbox[1]+10))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), zrgn)

    def test_Draw_Deer(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        for i in range(0, rgns.Count()):
            rgn = rgns.SelectObj(i)
            if rgn.Area() > 140000 and rgn.Area() < 150000:
                break

        startTime = time.perf_counter()
        r, image = rgn.Draw(image, (255, 0, 0, 64))
        endTime = time.perf_counter()
        self.assertEqual(0, r)
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveImage(self.id(), image)

    def test_Affine_Deer(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        for i in range(0, rgns.Count()):
            rgn = rgns.SelectObj(i)
            if rgn.Area() > 140000 and rgn.Area() < 150000:
                break

        affMat = mvlab.HomoMat2d_GenIdentity()
        affMat = mvlab.HomoMat2d_Rotate(affMat, 30, rgn.Centroid())
        affMat = mvlab.HomoMat2d_SlantLocal(affMat, 30, 'x')

        startTime = time.perf_counter()
        rgn = rgn.AffineTrans(affMat)
        endTime = time.perf_counter()

        bbox = rgn.BoundingBox()
        rgn = rgn.Move((-bbox[0]+10, -bbox[1]+10))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)

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

if __name__ == '__main__':
    unittest.main()