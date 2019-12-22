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

    def test_WKT_Polygon(self):
        contr = mvlab.Contour_ReadWkt("POLYGON((0 0,0 7,4 2,2 0,0 0))")
        self.assertAlmostEqual(contr.Area(), 16)
        self.assertFalse(contr.TestSelfIntersection('false'))
        extradata.SaveContours(self.id(), [contr.Zoom((50, 50))])

    def test_WKT_Polygon(self):
        contr = mvlab.Contour_ReadWkt("MULTIPOLYGON (((30 20, 45 40, 10 40, 30 20)),((15 5, 40 10, 10 20, 5 10, 15 5)))")
        extradata.SaveContours(self.id(), [contr.Zoom((5, 5))])

if __name__ == '__main__':
    unittest.main()
