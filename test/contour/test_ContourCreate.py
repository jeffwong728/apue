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

    def test_Create_CircleArc_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenCircleSector((1000, 1000), 500, 10, 300, 5, 'negative arc')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_CircleSlice_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenCircleSector((1000, 1000), 500, 10, 300, 5, 'negative slice')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_CircleChord_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenCircleSector((1000, 1000), 500, 10, 300, 5, 'negative chord')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_Ellipse_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenEllipse((1000, 1000), (500, 300), 5, 'negative')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_EllipseArc_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenEllipseSector((1000, 1000), (500, 300), 300, 10, 5, 'negative arc')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_EllipseSlice_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenEllipseSector((1000, 1000), (500, 300), 300, 10, 5, 'negative slice')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_EllipseChord_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenEllipseSector((1000, 1000), (500, 300), 300, 10, 5, 'negative chord')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_RotatedEllipse_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 5, 'positive')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_RotatedEllipseArc_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenRotatedEllipseSector((1000, 1000), (500, 300), 60, 60, 30, 5, 'negative arc')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_RotatedEllipseSlice_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenRotatedEllipseSector((1000, 1000), (500, 300), -60, 300, 250, 5, 'negative slice')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_RotatedEllipseChord_Contour(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenRotatedEllipseSector((1000, 1000), (500, 300), 200, 300, 10, 5, 'negative chord')
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

if __name__ == '__main__':
    unittest.main()
