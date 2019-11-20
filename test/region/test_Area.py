import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionArea(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Empty_Region(self):
        rgn = mvlab.Region_GenEmpty()
        self.assertAlmostEqual(rgn.Area(), 0.0, 'Empty region area {0:f} not 0.0'.format(rgn.Area()))

    def test_Rectangle_Region(self):
        rgn = mvlab.Region_GenRectangle((0, 0, 10, 10))
        self.assertAlmostEqual(rgn.Area(), 100.0, 'Rectangle region area {0:f} not 100.0'.format(rgn.Area()))

    def test_Mista_Area(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        r, rgns = rgn.Connect()

        startTime = time.perf_counter()
        area = rgn.Area()
        endTime = time.perf_counter()

        self.assertEqual(len(rgns), 941, 'Mista component number error')

        areaSum = numpy.sum([r.Area() for r in rgns])
        self.assertAlmostEqual(rgn.Area(), areaSum, 'Region area {0:f} not equal sum of compoent areas {1:f}'.format(rgn.Area(), areaSum))

if __name__ == '__main__':
    unittest.main()