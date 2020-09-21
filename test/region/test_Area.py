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
        self.assertAlmostEqual(rgn.Area(), 100.0, msg='Rectangle region area {0:f} not 100.0'.format(rgn.Area()))

    def test_Rectangle_BoundingBox(self):
        rgn = mvlab.Region_GenRectangle((0, 0, 10, 10))
        self.assertEqual(rgn.BoundingBox(), (0, 0, 10, 10), 'Rectangle bounding box error')

    def test_2Region_PointTouch(self):
        rgn1 = mvlab.Region_GenRectangle((10, 10, 10, 10))
        rgn2 = mvlab.Region_GenRectangle((20, 20, 10, 10))
        rgn = rgn1.Union2(rgn2)

        self.assertEqual(200, rgn.Area())
        self.assertEqual(1, rgn.GetContour().Count())
        self.assertEqual(rgn.Area(), rgn.GetContour().Area())

        extradata.SaveRegion(self.id(), rgn)

    def test_2Region_With_2Holes_PointTouch(self):
        rgn1 = mvlab.Region_GenRectangle((10, 10, 10, 10))
        rgn2 = mvlab.Region_GenRectangle((20, 20, 10, 10))
        rgn = rgn1.Union2(rgn2)

        rgn1 = mvlab.Region_GenRectangle((13, 13, 5, 5))
        rgn2 = mvlab.Region_GenRectangle((23, 23, 5, 5))
        rgn = rgn.Difference(rgn1.Union2(rgn2))

        self.assertEqual(150, rgn.Area())
        self.assertEqual(1, rgn.GetContour().Count())
        self.assertEqual(2, rgn.GetHole().Count())
        self.assertEqual(rgn.GetContour().Area(), rgn.Area()+rgn.GetHole().Area())

        extradata.SaveRegion(self.id(), rgn)

    def test_Mista_Area(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        startTime = time.perf_counter()
        area = rgns.Area()
        endTime = time.perf_counter()

        self.assertEqual(rgns.Count(), 941)

        areaSum = rgns.Area()
        self.assertAlmostEqual(rgn.Area(), areaSum, 'Region area {0:f} not equal sum of compoent areas {1:f}'.format(rgn.Area(), areaSum))

    def test_Mista_Hole_Area(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        self.assertAlmostEqual(rgn.GetHole().Area(), rgn.AreaHoles())

if __name__ == '__main__':
    unittest.main()