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

class TestContourCreate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_Create_Rotated_Rectangle(self):
        startTime = time.perf_counter()
        contr = mvlab.Contour_GenRotatedRectangle(((1000, 500), (500, 200), 10))
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

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

    def test_Create_PolygonRounded_Contour(self):
        startTime = time.perf_counter()
        points = [(100, 100), (50, 200), (150, 150), (250, 200), (200, 100)]
        contr = mvlab.Contour_GenPolygonRounded(points, [(10, 10), (5, 5), (5, 5), (5, 5), (10, 10)], 1)
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), [contr])

    def test_Create_Cross_Contour(self):
        points = []
        sizes = []
        angles = []
        random.seed()
        for i in range(0, 50):
            points.append((random.uniform(0, 640), random.uniform(0, 480)))
            sizes.append((10, 10))
            angles.append(random.uniform(0, 360))

        contr = mvlab.Contour_GenCross(points, sizes, angles)
        extradata.SaveContours(self.id(), [contr])

if __name__ == '__main__':
    unittest.main()
