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

class TestContourConvexHull(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Simple_Convex(self):
        contr = mvlab.Contour_GenPolygon([(100, 100), (100, 200), (150, 150), (200, 200), (200, 100)])
        hull = contr.GetConvex()
        self.assertEqual(4, hull.CountPoints())
        extradata.SaveContours(self.id(), [hull, contr])

    def test_Random_Polygon(self):
        n = int(random.uniform(1, 100))
        points = []
        for i in range(0, n):
            points.append((random.uniform(0, 640), random.uniform(0, 480)))

        plg = mvlab.Contour_GenPolygon(points)
        mvlab.SetGlobalOption('convexhullalgo', 'Sklansky')
        sklansky = plg.GetConvex()
        mvlab.SetGlobalOption('convexhullalgo', 'AndrewMonotoneChain')
        andrew = plg.GetConvex()

        self.assertEqual(sklansky.CountPoints(), andrew.CountPoints())
        self.assertAlmostEqual(sklansky.Area(), andrew.Area(), delta=0.1)

        pointConts = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        extradata.SaveContours(self.id(), [pointConts, sklansky, andrew])

if __name__ == '__main__':
    unittest.main()
