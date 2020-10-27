import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata
import math
import random

class TestContourConvexHullSklansky(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        mvlab.SetGlobalOption('convex_hull_method', 'Sklansky')

    def test_Simple_Convex(self):
        points = [(100, 100), (100, 200), (150, 150), (200, 200), (200, 100)]
        self.DoConvexTest(points, (200, 0), 4)

    def test_Self_InterSection(self):
        points = [(553.5009213982449, 279.6791014058838), (451.74808216792206, 471.55617128915424)]
        points += [(172.6363981839969, 446.95908564373246), (570.6216609786668, 325.79701326342314)]
        points += [(455.9614682034474, 406.1973146695654)]
        self.DoConvexTest(points, (0, 200), 4, 0.1)

    def test_Points_Coincidence(self):
        points = [(150, 100), (100, 100), (100, 150), (100, 200), (100, 200), (150, 200), (200, 200), (200, 150), (200, 100)]
        self.DoConvexTest(points, (200, 0), 4)

    def test_Identical_X(self):
        points = [(100, 100), (100, 200), (100, 300), (100, 400), (100, 500)]
        self.DoConvexTest(points, (20, 0), 2)

    def test_Identical_Y(self):
        points = [(100, 100), (200, 100), (300, 100), (400, 100), (450, 100), (500, 100)]
        self.DoConvexTest(points, (0, 20), 2)

    def test_Collinear(self):
        points = [(100, 100), (200, 200), (300, 300), (400, 400), (400, 400), (500, 500)]
        self.DoConvexTest(points, (20, 0), 2)

    def test_Trapezoid(self):
        points = [(100, 100), (200, 100), (250, 200), (50, 200)]
        self.DoConvexTest(points, (220, 0), 4)

    def test_Multiple_Collinear_I(self):
        points = [(100, 100), (130, 100), (160, 100), (200, 100)]
        points += [(280, 180), (250, 150), (300, 200)]
        points += [(200, 200), (150, 200), (100, 200), (50, 200)]
        points += [(60, 180), (90, 120)]
        self.DoConvexTest(points, (0, 120), 4)

    def test_Multiple_Collinear_II(self):
        points = [(100, 100), (130, 100), (160, 100), (200, 100)]
        points += [(280, 180), (250, 150), (300, 200)]
        points += [(200, 200), (150, 200), (100, 200), (50, 200)]
        points += [(60, 180), (90, 120)]

        affMat = mvlab.HomoMat2d_GenIdentity()
        affMat = mvlab.HomoMat2d_Rotate(affMat, 90, (150, 150))
        points = [mvlab.HomoMat2d_AffineTransPoint2d(affMat, point) for point in points]
        self.DoConvexTest(points, (120, 0), 4, tol=0.01)

    def DoConvexTest(self, points, delta, numVertices, tol=None):
        contr1 = mvlab.Contour_GenPolygon(points)
        crosses1 = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))

        points.reverse()
        points = [(x+delta[0], y+delta[1]) for (x, y) in points]
        contr2 = mvlab.Contour_GenPolygon(points)
        crosses2 = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))

        hull1 = contr1.GetConvex()
        hull2 = contr2.GetConvex()

        extradata.SaveContours(self.id(), [hull1, hull2, crosses1, crosses2])

        self.assertEqual(numVertices, hull1.CountPoints())
        self.assertEqual(numVertices, hull2.CountPoints())
        self.assertAlmostEqual(hull1.Area(), hull2.Area(), delta=tol)
        self.assertAlmostEqual(hull1.Length(), hull2.Length(), delta=tol)

        print("Area: ({0:f}, {1:f}) ".format(hull1.Area(), hull2.Area()))
        print("Perimeter: ({0:f}, {1:f})".format(hull1.Length(), hull2.Length()))

if __name__ == '__main__':
    unittest.main()
