import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Empty(self):
        rgn = mvlab.Region_CreateEmpty()
        self.assertTrue(rgn.Empty())

    def test_Not_Empty(self):
        rgn = mvlab.Region_CreateCircle((20, 20), 10)
        self.assertFalse(rgn.Empty())

    def test_Empty_Equal_Empty(self):
        rgn1 = mvlab.Region_CreateEmpty()
        rgn2 = mvlab.Region_CreateEmpty()
        self.assertTrue(rgn1.TestEqual(rgn2))

    def test_Empty_Not_Equal_Circle(self):
        rgn1 = mvlab.Region_CreateEmpty()
        rgn2 = mvlab.Region_CreateCircle((20, 20), 10)
        self.assertFalse(rgn1.TestEqual(rgn2))

    def test_Circle_Not_Equal_Empty(self):
        rgn1 = mvlab.Region_CreateCircle((20, 20), 10)
        rgn2 = mvlab.Region_CreateEmpty()
        self.assertFalse(rgn1.TestEqual(rgn2))

    def test_Circle_Equal_Circle(self):
        rgn1 = mvlab.Region_CreateCircle((20, 20), 10)
        rgn2 = mvlab.Region_CreateCircle((20, 20), 10)
        self.assertTrue(rgn1.TestEqual(rgn2))

    def test_Circles_Not_Equal(self):
        rgn1 = mvlab.Region_CreateCircle((20, 20), 10)
        rgn2 = mvlab.Region_CreateCircle((20, 20), 11)
        self.assertFalse(rgn1.TestEqual(rgn2))

if __name__ == '__main__':
    unittest.main()