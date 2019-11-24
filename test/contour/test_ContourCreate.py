import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestContourCreate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Create_Circle_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenCircle((1000, 1000), 500, 5, 'negative')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_Ellipse_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenEllipse((1000, 1000), (500, 300), 5, 'negative')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

if __name__ == '__main__':
    unittest.main()
