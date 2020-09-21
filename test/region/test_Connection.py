import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

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
        rgns = rgn.Connect()

        self.assertEqual(rgns.Count(), 2, '2Box component number error')
        self.assertAlmostEqual(rgn.Area(), rgns.Area())

    def test_2HBox_Connection(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 1:31] = 255
        image[10:15, 33:63] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        rgns = rgn.Connect()

        self.assertEqual(rgns.Count(), 2, '2Box component number error')
        self.assertAlmostEqual(rgn.Area(), rgns.Area())

    def test_Scrach_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'scrach.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        rgns = rgn.Connect()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegions(self.id(), [rgns.SelectObj(i) for i in range(0, rgns.Count())], image.shape)

        self.assertEqual(rgns.Count(), 95, 'Scrach component number error')

    def test_Mista_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)

        startTime = time.perf_counter()
        rgns = rgn.Connect()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegions(self.id(), [rgns.SelectObj(i) for i in range(0, rgns.Count())], image.shape)

        self.assertEqual(rgns.Count(), 941, 'Mista component number error')

    def test_Digits_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 151, 255)

        startTime = time.perf_counter()
        rgns = rgn.Connect()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegions(self.id(), [rgns.SelectObj(i) for i in range(0, rgns.Count())], image.shape)

        self.assertEqual(rgns.Count(), 5584, 'Digits component number error')

    def test_PCB_Layout_Connection(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 0, 50)

        startTime = time.perf_counter()
        rgns = rgn.Connect()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        extradata.SaveRegions(self.id(), [rgns.SelectObj(i) for i in range(0, rgns.Count())], image.shape)

        self.assertEqual(rgns.Count(), 49, 'pcb_layout.png component number error')

if __name__ == '__main__':
    unittest.main()
