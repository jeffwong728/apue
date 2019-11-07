import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging

class TestRegionConnection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_2VBox_Connection(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 20:50] = 255
        image[20:25, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        rgns = rgn.Connect(8)

        self.assertEqual(rgns.Count(), 2, '2Box component number error')
        self.assertAlmostEqual(rgn.Area(), numpy.sum(rgns.Area()))

    def test_2HBox_Connection(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 1:31] = 255
        image[10:15, 33:63] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        rgns = rgn.Connect(8)

        self.assertEqual(rgns.Count(), 2, '2Box component number error')
        self.assertAlmostEqual(rgn.Area(), numpy.sum(rgns.Area()))

    def test_2VBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 20:50] = 255
        image[20:25, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outers = rgn.OuterContours()

        self.assertEqual(len(outers), 2, '2Box contour number error')
        self.assertAlmostEqual(cv2.contourArea(outers[0].Simplify(1.0)[1]), 150.0)
        self.assertAlmostEqual(cv2.contourArea(outers[1].Simplify(1.0)[1]), 150.0)

    def test_2HBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 1:31] = 255
        image[10:15, 33:63] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outers = rgn.OuterContours()

        self.assertEqual(len(outers), 2, '2Box contour number error')
        self.assertAlmostEqual(cv2.contourArea(outers[0].Simplify(1.0)[1]), 150.0)
        self.assertAlmostEqual(cv2.contourArea(outers[1].Simplify(1.0)[1]), 150.0)

    def test_Simple_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:11, 1:10] = 255
        image[10:11, 15:30] = 255
        image[11:12, 5:35] = 255
        image[30:31, 1:30] = 255
        image[31:32, 5:10] = 255
        image[31:32, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        r, outers = rgn.OuterContours()

        self.assertEqual(len(outers), 2, 'Simple contour number error')

if __name__ == '__main__':
    unittest.main()
