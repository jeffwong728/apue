import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestContour(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_2VBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 20:50] = 255
        image[20:25, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        outer = rgn.GetContour()

        self.assertEqual(r, 0, '2Box contour number error')
        self.assertAlmostEqual(outer.Simplify(1.0).Area(), rgn.Area())

    def test_2HBox_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:15, 1:31] = 255
        image[10:15, 33:63] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        outer = rgn.GetContour()

        self.assertEqual(r, 0, '2Box contour number error')
        self.assertAlmostEqual(outer.Simplify(1.0).Area(), rgn.Area())

    def test_Contour_Points(self):
        image = numpy.zeros((48, 64, 1), numpy.uint8)
        image[10:11, 30:35] = 255
        image[11:12, 5:60]  = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        outer = rgn.GetContour()
        points = outer.GetPoints()

        for point in points:
            print('({0:.1f}, {1:.1f})'.format(point[0], point[1]))

        self.assertEqual(len(points), 8)
        extradata.SaveContours(self.id(), [outer])

    def test_Simple_Contour(self):
        image = numpy.zeros((32, 64, 1), numpy.uint8)
        image[10:11, 1:10] = 255
        image[10:11, 15:30] = 255
        image[11:12, 5:35] = 255
        image[30:31, 1:30] = 255
        image[31:32, 5:10] = 255
        image[31:32, 20:50] = 255
        r, rgn = mvlab.Threshold(image, 150, 255)
        outer = rgn.GetContour()

        self.assertEqual(r, 0, 'Simple contour number error')

    def test_Scrach_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()
        startTime = time.perf_counter()
        outers = rgns.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'scrach.png' error")

    def test_Mista_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        startTime = time.perf_counter()
        outers = rgns.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'mista.png' error")

    def test_Digits_Contour(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'digits.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 151, 255)
        rgns = rgn.Connect()
        startTime = time.perf_counter()
        outers = rgns.GetContour()
        endTime = time.perf_counter()
        extradata.SavePerformanceData(self.id(), (endTime-startTime))

        self.assertEqual(r, 0, "Contour 'digits.png' error")

    def test_Contour_Simplify(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)

        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        for i in range(0, rgns.Count()):
           rgn = rgns.SelectObj(i)
           if rgn.Area() > 100000 and rgn.Area() < 120000:
               break

        bbox = rgn.BoundingBox()
        srgn = rgn.Move((-bbox[0]+10, -bbox[1]+10))
        souter = srgn.GetContour()
        print("Before Simplify: {0:d}".format(souter.CountPoints()), end=os.linesep)

        startTime = time.perf_counter()
        souter = souter.Simplify(1)
        endTime = time.perf_counter()
        print("After Simplify: {0:d}".format(souter.CountPoints()), end=os.linesep)

        extradata.SavePerformanceData(self.id(), (endTime-startTime))
        rgn = rgn.Move((-bbox[0]+10, -bbox[1]+bbox[3]+20))
        extradata.SaveContours(self.id(), [souter, rgn.GetContour()])

    def test_Draw_Ellipse(self):
        image = numpy.zeros((480, 640), numpy.uint8)
        points = [[100.5, 30.5], [40.5, 80.5], [10.5, 90.5]]
        for point in points:
            point[0] = int(round(point[0]*1024))
            point[1] = int(round(point[1]*1024))
        triangle = numpy.array(points, numpy.int32)
        cv2.fillConvexPoly(image, triangle, (255, 255, 255, 255), 16, 10)
        extradata.SaveImage(self.id(), image)

if __name__ == '__main__':
    unittest.main()
