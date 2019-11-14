import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionCreate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Create_Empty(self):
        rgn = mvlab.Region_CreateEmpty()
        self.assertAlmostEqual(rgn.Area(), 0.0, 'Empty region area {0:f} not 0.0'.format(rgn.Area()))

    def test_Create_Rectangle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_CreateRectangle((10, 10, 1000, 1000))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)

        self.assertAlmostEqual(rgn.Area(), 1e6, msg='Rectangle region area {0:f} not {1:f}'.format(rgn.Area(), 1e6))

    def test_Create_Circle(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_CreateCircle((1250, 1250), 750)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)

        x, y = rgn.Centroid()
        self.assertAlmostEqual(x, 1250, places=2)
        self.assertAlmostEqual(y, 1250, places=2)

    def test_Create_Polygon(self):
        startTime = time.perf_counter()
        rgn = mvlab.Region_CreateCircle((1250, 1250), 750)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveRegion(self.id(), rgn)

        x, y = rgn.Centroid()
        self.assertAlmostEqual(x, 1250, places=2)
        self.assertAlmostEqual(y, 1250, places=2)

if __name__ == '__main__':
    unittest.main()