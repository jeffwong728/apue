import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestStructuringElement(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Square(self):
        se = mvlab.Region_GenStructuringElement('square', 1)
        self.assertEqual(se.Area(), 9.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-1, -1, 3, 3), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        se = mvlab.Region_GenStructuringElement('square', 0)
        self.assertEqual(se.Area(), 25.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-2, -2, 5, 5), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        se = mvlab.Region_GenStructuringElement('square', 7)
        self.assertEqual(se.Area(), 225.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-7, -7, 15, 15), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        extradata.SaveRegion(self.id(), se.Move((7, 7)))

    def test_Circle(self):
        se = mvlab.Region_GenStructuringElement('circle', 1)
        self.assertEqual(se.Area(), 9.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-1, -1, 3, 3), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        se = mvlab.Region_GenStructuringElement('circle', 2)
        self.assertEqual(se.Area(), 21.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-2, -2, 5, 5), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        se = mvlab.Region_GenStructuringElement('circle', 5)
        self.assertEqual(se.Area(), 97.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-5, -5, 11, 11), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        se = mvlab.Region_GenStructuringElement('circle', 7)
        self.assertEqual(se.Area(), 177.0, msg='Structuring element area: {0:f}'.format(se.Area()))
        self.assertEqual(se.BoundingBox(), (-7, -7, 15, 15), 'Structuring element bbox: {0}'.format(se.BoundingBox()))

        extradata.SaveRegion(self.id(), se.Move((7, 7)))

    def test_Diamond(self):
        se = mvlab.Region_GenStructuringElement('diamond', 1)
        self.assertEqual(se.BoundingBox(), (-1, -1, 3, 3), 'Structuring element bbox: {0}'.format(se.BoundingBox()))
        self.verifyRegionIntegrity(se)

        se = mvlab.Region_GenStructuringElement('diamond', 2)
        self.assertEqual(se.BoundingBox(), (-2, -2, 5, 5), 'Structuring element bbox: {0}'.format(se.BoundingBox()))
        self.verifyRegionIntegrity(se)

        se = mvlab.Region_GenStructuringElement('diamond', 5)
        self.assertEqual(se.BoundingBox(), (-5, -5, 11, 11), 'Structuring element bbox: {0}'.format(se.BoundingBox()))
        self.verifyRegionIntegrity(se)

        extradata.SaveRegion(self.id(), se.Move((5, 5)))

    def test_HLine(self):
        se = mvlab.Region_GenStructuringElement('hline', 5)
        self.assertEqual(se.BoundingBox(), (-5, 0, 11, 1), 'Structuring element bbox: {0}'.format(se.BoundingBox()))
        self.verifyRegionIntegrity(se)
        extradata.SaveRegion(self.id(), se.Move((5, 0)))

    def test_VLine(self):
        se = mvlab.Region_GenStructuringElement('vline', 5)
        self.assertEqual(se.BoundingBox(), (0, -5, 1, 11), 'Structuring element bbox: {0}'.format(se.BoundingBox()))
        self.verifyRegionIntegrity(se)
        extradata.SaveRegion(self.id(), se.Move((0, 5)))

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