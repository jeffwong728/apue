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

class TestRegionCreate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_Create_Empty(self):
        rgn = mvlab.Region_GenEmpty()
        self.assertAlmostEqual(rgn.Area(), 0.0, 'Empty region area {0:f} not 0.0'.format(rgn.Area()))
        self.verifyRegionIntegrity(rgn)

    def test_Create_Rectangle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenRectangle((10, 10, 1000, 1000))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        self.assertAlmostEqual(rgn.Area(), 1e6, msg='Rectangle region area {0:f} not {1:f}'.format(rgn.Area(), 1e6))

    def test_Create_RotatedRectangle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenRotatedRectangle(((1000, 1000), (500, 800), 30))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        self.assertAlmostEqual(rgn.Centroid()[0], 1000, delta=1)
        self.assertAlmostEqual(rgn.Centroid()[1], 1000, delta=1)

    def test_Create_Checker(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenChecker((1920, 1200), (95, 95))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((5, 5), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 25)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((5, 10), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 40)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((10, 5), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 40)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((8, 8), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 64)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((16, 16), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 128)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenChecker((15, 15), (8, 8))
        self.assertAlmostEqual(rgn.Area(), 113)
        self.verifyRegionIntegrity(rgn)

    def test_Create_Circle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenCircle((1250, 1250), 750.5)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        x, y = rgn.Centroid()
        self.assertEqual(x, 1250)
        self.assertEqual(y, 1250)

        rgn = mvlab.Region_GenCircle((10, 10), 0.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10, 10), 0.5): ')
        self.dumpRegion(rgn)

        rgn = mvlab.Region_GenCircle((10.5, 10.5), 0.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10.5, 10.5), 0.5): ')
        self.dumpRegion(rgn)

        rgn = mvlab.Region_GenCircle((10.5, 10.5), 1.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10.5, 10.5), 1.5): ')
        self.dumpRegion(rgn)

        rgn = mvlab.Region_GenCircle((10.5, 10.5), 2.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10.5, 10.5), 2.5): ')
        self.dumpRegion(rgn)

        rgn = mvlab.Region_GenCircle((10.5, 10.5), 3.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10.5, 10.5), 3.5): ')
        self.dumpRegion(rgn)

        rgn = mvlab.Region_GenCircle((10.25, 10.25), 0.5)
        self.verifyRegionIntegrity(rgn)
        print('Circle((10.25, 10.25), 0.5): ')
        self.dumpRegion(rgn)

    def test_Create_Random_Circle(self):
        r = random.uniform(1, 640) / 2.0
        x = random.uniform(-320, 320)
        y = random.uniform(-320, 320)
        rgn = mvlab.Region_GenCircle((x, y), r)

        print("x={0}, y={1}, r={2}".format(x, y, r))
        extradata.SaveRegion(self.id(), rgn.Move((int(-x+round(r))+5, int(-y+round(r))+5)))
        self.verifyRegionIntegrity(rgn)

    def test_Create_CircleSector(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenCircleSector((1250, 1250), 750.5, 350, 300)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

    def test_Create_Ellipse(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenEllipse((1250, 1250), (750.5, 550.5))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        x, y = rgn.Centroid()
        self.assertAlmostEqual(x, 1250)
        self.assertAlmostEqual(y, 1250)

    def test_Create_RotatedEllipse(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenRotatedEllipse((1250, 1250), (750.5, 550.5), 30)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

        x, y = rgn.Centroid()
        self.assertAlmostEqual(x, 1250, delta=1)
        self.assertAlmostEqual(y, 1250, delta=1)

    def test_Create_EllipseSector(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenEllipseSector((1250, 1250), (750.5, 550.5), 30, 350, 300)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

    def test_Create_Triangle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenTriangle((1500, 100), (700, 700), (1900, 1000))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 100), (100, 100), (100, 100))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 100), (100, 100), (100, 200))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 10), (50, 100), (150, 100))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 10), (150, 100), (50, 100))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 200), (50, 100), (150, 100))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 200), (150, 100), (50, 100))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 100), (200, 200), (150, 300))
        self.verifyRegionIntegrity(rgn)

        rgn = mvlab.Region_GenTriangle((100, 100), (100, 200), (50, 200))
        self.verifyRegionIntegrity(rgn)

        for n in range(0, 50):
            v1 = (random.uniform(0, 640), random.uniform(0, 640))
            v2 = (random.uniform(0, 640), random.uniform(0, 640))
            v3 = (random.uniform(0, 640), random.uniform(0, 640))
            rgn = mvlab.Region_GenTriangle(v1, v2, v3)
            self.verifyRegionIntegrity(rgn)
        extradata.SaveRegion(self.id(), rgn)

    def test_Create_Quadrangle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_GenQuadrangle((1500, 100), (700, 700), (1900, 1000), (2000, 500))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.verifyRegionIntegrity(rgn)

        for n in range(0, 50):
            v1 = (random.uniform(0, 640), random.uniform(0, 640))
            v2 = (random.uniform(0, 640), random.uniform(0, 640))
            v3 = (random.uniform(0, 640), random.uniform(0, 640))
            v4 = (random.uniform(0, 640), random.uniform(0, 640))
            rgn = mvlab.Region_GenQuadrangle(v1, v2, v3, v4)
            self.verifyRegionIntegrity(rgn)
        extradata.SaveRegion(self.id(), rgn)

    def test_Create_Polygon_Zero(self):
        rgn = mvlab.Region_GenPolygon([(100, 50), (100, 50), (100, 50), (100, 50), (100, 50), (100, 50), (100, 50)])
        self.verifyRegionIntegrity(rgn)
        self.assertEqual(0, rgn.CountRuns())

    def test_Create_Polygon_Line(self):
        rgn = mvlab.Region_GenPolygon([(100, 100), (90, 90), (80, 80), (10, 10), (1, 1)])
        self.verifyRegionIntegrity(rgn)
        self.assertEqual(99, rgn.CountRuns())
        extradata.SaveRegion(self.id(), rgn)

    def test_Create_Polygon(self):
        rgn = mvlab.Region_GenPolygon([(100, 50), (50, 150), (100, 200), (150, 180), (200, 200), (100, 150), (150, 100)])
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

    def test_Create_Polygon_Bull(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        for i in range(0, rgns.Count()):
            rgn = rgns.SelectObj(i)
            if rgn.Area() > 140000 and rgn.Area() < 150000:
                break

        bbox = rgn.BoundingBox()
        srgn = rgn.Move((-bbox[0]+10, -bbox[1]+10))
        souter = srgn.GetContour()
        souter = souter.Simplify(1)

        points = souter.GetPoints()
        points = [point for point in points]

        startTime = time.perf_counter()
        rgn = mvlab.Region_GenPolygon(points)
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)
        self.verifyRegionIntegrity(rgn)

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

if __name__ == '__main__':
    unittest.main()