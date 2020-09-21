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

class TestContourWkt(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_WKT_Ring(self):
        contr = mvlab.Contour_ReadWkt("POLYGON((0 0,0 7,4 2,2 0,0 0))")
        self.assertAlmostEqual(contr.Area(), 16)
        self.assertTrue(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((50, 50))])

    def test_WKT_Polygon(self):
        contr = mvlab.Contour_ReadWkt("POLYGON((35 10, 45 45, 15 40, 10 20, 35 10), (20 30, 35 35, 30 20, 20 30))")
        self.assertTrue(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

    def test_WKT_Multi_Polygon(self):
        contr = mvlab.Contour_ReadWkt("MULTIPOLYGON (((30 20, 45 40, 10 40, 30 20)), ((15 5, 40 10, 10 20, 5 10, 15 5)))")
        self.assertTrue(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

    def test_WKT_Multi_Polygon_With_Hole(self):
        contr = mvlab.Contour_ReadWkt("MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)), ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20)))")
        self.assertFalse(contr.TestPoint((30, 30)))
        self.assertTrue(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

    def test_WKT_LineString(self):
        contr = mvlab.Contour_ReadWkt("LINESTRING (30 10, 10 30, 40 40)")
        self.assertFalse(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

    def test_WKT_Multi_LineString(self):
        contr = mvlab.Contour_ReadWkt("MULTILINESTRING ((10 10, 20 20, 10 40), (40 40, 30 30, 40 20, 30 10))")
        self.assertFalse(contr.TestClosed())
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

if __name__ == '__main__':
    unittest.main()
