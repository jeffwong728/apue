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
        rgn = mvlab.Region_GenEmpty()
        self.assertTrue(rgn.Empty())

    def test_Not_Empty(self):
        rgn = mvlab.Region_GenCircle((20, 20), 10)
        self.assertFalse(rgn.Empty())

    def test_Empty_Equal_Empty(self):
        rgn1 = mvlab.Region_GenEmpty()
        rgn2 = mvlab.Region_GenEmpty()
        self.assertTrue(rgn1.TestEqual(rgn2))

    def test_Empty_Not_Equal_Circle(self):
        rgn1 = mvlab.Region_GenEmpty()
        rgn2 = mvlab.Region_GenCircle((20, 20), 10)
        self.assertFalse(rgn1.TestEqual(rgn2))

    def test_Circle_Not_Equal_Empty(self):
        rgn1 = mvlab.Region_GenCircle((20, 20), 10)
        rgn2 = mvlab.Region_GenEmpty()
        self.assertFalse(rgn1.TestEqual(rgn2))

    def test_Circle_Equal_Circle(self):
        rgn1 = mvlab.Region_GenCircle((20, 20), 10)
        rgn2 = mvlab.Region_GenCircle((20, 20), 10)
        self.assertTrue(rgn1.TestEqual(rgn2))

    def test_Circles_Not_Equal(self):
        rgn1 = mvlab.Region_GenCircle((20, 20), 10)
        rgn2 = mvlab.Region_GenCircle((20, 20), 11)
        self.assertFalse(rgn1.TestEqual(rgn2))

    def test_Point_Outside_Empty_Region(self):
        rgn = mvlab.Region_GenEmpty()
        self.assertFalse(rgn.TestPoint((20, 28)))

    def test_Point_Inside_Circle(self):
        rgn = mvlab.Region_GenCircle((20, 20), 10)
        self.assertTrue(rgn.TestPoint((20, 28)))

    def test_Point_Outside_Circle(self):
        rgn = mvlab.Region_GenCircle((20, 20), 10)
        self.assertFalse(rgn.TestPoint((20, 32)))

    def test_Point_Inside_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        r = rgn.TestPoint((1311, 1939))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertTrue(r)

    def test_Point_Outside_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        r = rgn.TestPoint((733, 1660))
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertFalse(r)

    def test_Empty_Region_Inside_Any_Region(self):
        rgn = mvlab.Region_GenEmpty()
        rgn2 = mvlab.Region_GenCircle((1311, 1939), 25)
        self.assertTrue(rgn2.TestSubset(rgn))
        self.assertTrue(rgn.TestSubset(rgn))
        self.assertTrue(rgn2.TestSubset(rgn2))
        self.assertFalse(rgn.TestSubset(rgn2))

    def test_Region_Inside_Region(self):
        image = numpy.zeros((48, 64, 1), numpy.uint8)
        image[10:20, 10:15] = 255
        image[10:20, 20:25] = 255
        image[10:20, 30:35] = 255
        r, rgn1 = mvlab.Threshold(image, 150, 255)

        image[:, :] = 0
        image[10:20, 10:35] = 255
        r, rgn2 = mvlab.Threshold(image, 150, 255)
        self.assertTrue(rgn2.TestSubset(rgn1))

    def test_Region_Outside_Region(self):
        image = numpy.zeros((48, 64, 1), numpy.uint8)
        image[10:20, 10:15] = 255
        image[10:20, 20:25] = 255
        image[10:20, 30:35] = 255
        r, rgn1 = mvlab.Threshold(image, 150, 255)

        image[:, :] = 0
        image[10:20, 11:35] = 255
        r, rgn2 = mvlab.Threshold(image, 150, 255)
        self.assertFalse(rgn2.TestSubset(rgn1))

    def test_Region_Inside_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)
        rgn2 = mvlab.Region_GenCircle((1311, 1939), 25)

        startTime = time.perf_counter()
        r = rgn.TestSubset(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertTrue(r)

    def test_Region_Outside_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'), cv2.IMREAD_UNCHANGED)
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)
        rgn2 = mvlab.Region_GenCircle((733, 1660), 10)

        startTime = time.perf_counter()
        r = rgn.TestSubset(rgn2)
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        self.assertFalse(r)

if __name__ == '__main__':
    unittest.main()