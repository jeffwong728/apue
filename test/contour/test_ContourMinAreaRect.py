import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata
import math
import random

class TestContourMinAreaRect(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        mvlab.SetGlobalOption('convex_hull_method', 'Andrew')
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_All_Points_Coincidence_MinAreaRect(self):
        points = [(300, 300)]*10
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 300.0)
        self.assertAlmostEqual(minRect[0][1], 300.0)

    def test_Identical_X_MinAreaRect(self):
        points = [(100, 100), (100, 200), (100, 300), (100, 400), (100, 500)]
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 100.0)
        self.assertAlmostEqual(minRect[0][1], 300.0)

    def test_Identical_Y_MinAreaRect(self):
        points = [(100, 100), (200, 100), (300, 100), (400, 100), (450, 100), (500, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        minRect = plg.SmallestRectangle()
        self.assertAlmostEqual(minRect[0][0], 300.0)
        self.assertAlmostEqual(minRect[0][1], 100.0)

    def test_Triangle_MinAreaRect_CW(self):
        points = [(50, 200), (100, 100), (150, 200)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 100.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 100.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 180.0)

    def test_Triangle_MinAreaRect_CCW(self):
        points = [(50, 200), (150, 200), (100, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 115.0)
        self.assertAlmostEqual(minRect[0][1], 170.0)
        self.assertAlmostEqual(minRect[1][0], 111.80339813232422)
        self.assertAlmostEqual(minRect[1][1], 89.44271850585938)
        self.assertAlmostEqual(minRect[2], 116.5650405883789)

    def test_Trapezoid_MinAreaRect_CW(self):
        points = [(100, 100), (200, 100), (250, 200), (50, 200)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 150.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 200.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 0.0)

    def test_Trapezoid_MinAreaRect_CCW(self):
        points = [(100, 100), (50, 200), (250, 200), (200, 100)]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 150.0)
        self.assertAlmostEqual(minRect[0][1], 150.0)
        self.assertAlmostEqual(minRect[1][0], 200.0)
        self.assertAlmostEqual(minRect[1][1], 100.0)
        self.assertAlmostEqual(minRect[2], 180.0)

    def test_RotatedRectangle_MinAreaRect(self):
        contr = mvlab.Contour_GenRotatedRectangle(((100, 100), (50, 30), 30))
        minRect = contr.SmallestRectangle()

        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 100, delta=0.001)
        self.assertAlmostEqual(minRect[0][1], 100, delta=0.001)
        self.assertAlmostEqual(minRect[1][0], 50, delta=0.001)
        self.assertAlmostEqual(minRect[1][1], 30, delta=0.001)
        self.assertAlmostEqual(minRect[2], -150.0, delta=0.001)

    def test_RotatedEllipse_MinAreaRect_CW(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 2, 'negative')
        startTime = time.perf_counter()
        minRect = contr.SmallestRectangle()
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[0][1], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][1], 600, delta=0.1)
        self.assertAlmostEqual(minRect[2], -150.0, delta=0.1)

    def test_RotatedEllipse_MinAreaRect_CCW(self):
        contr = mvlab.Contour_GenRotatedEllipse((1000, 1000), (500, 300), 30, 2, 'positive')
        startTime = time.perf_counter()
        minRect = contr.SmallestRectangle()
        endTime = time.perf_counter()

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        rcontr = mvlab.Contour_GenRotatedRectangle(minRect)
        extradata.SaveContours(self.id(), [contr, rcontr])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[0][1], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][0], 1000, delta=0.1)
        self.assertAlmostEqual(minRect[1][1], 600, delta=0.1)
        self.assertAlmostEqual(minRect[2], -150.0, delta=0.1)

    def test_Convex_Polygon_MinAreaRect(self):
        points = [(250.22852, 16.357777), (444.14212, 1.1392639), (622.03546, 38.92456)]
        points += [(543.08356, 340.36865), (483.9272, 436.66464), (386.01123, 475.3857)]
        points += [(43.13343, 361.93884), (6.043603, 324.05008), (0.8927474, 42.904503), (8.817269, 38.88925)]
        points = [(x+150, y+150) for (x, y) in points]
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*len(points), [0]*len(points))
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 425.828857421875)
        self.assertAlmostEqual(minRect[0][1], 358.6856689453125)
        self.assertAlmostEqual(minRect[1][0], 606.7611083984375)
        self.assertAlmostEqual(minRect[1][1], 475.9751892089844)
        self.assertAlmostEqual(minRect[2], 11.991644859313965)

    def test_Star_MinAreaRect(self):
        plg = extradata.LoadTextPolygon('star_nodes.txt')
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [plg.Move((2, 2)).Zoom((50, 50)), mvlab.Contour_GenRotatedRectangle(minRect).Move((2, 2)).Zoom((50, 50))])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 0.056128501892089844)
        self.assertAlmostEqual(minRect[0][1], -0.0772542953491211)
        self.assertAlmostEqual(minRect[1][0], 1.9021129608154297)
        self.assertAlmostEqual(minRect[1][1], 1.8090171813964844)
        self.assertAlmostEqual(minRect[2], -144.0)

    def test_Hand_MinAreaRect(self):
        plg = extradata.LoadTextPolygon('hand_nodes.txt')
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [plg.Move((1, 1)).Zoom((200, 200)), mvlab.Contour_GenRotatedRectangle(minRect).Move((1, 1)).Zoom((200, 200))])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 0.4325481653213501)
        self.assertAlmostEqual(minRect[0][1], 0.5498473644256592)
        self.assertAlmostEqual(minRect[1][0], 0.688842236995697)
        self.assertAlmostEqual(minRect[1][1], 0.3747103214263916)
        self.assertAlmostEqual(minRect[2], -72.26773071289062)

    def test_Comb_MinAreaRect(self):
        plg = extradata.LoadTextPolygon('comb_nodes.txt')
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [plg.Move((1, 5)).Zoom((50, 50)), mvlab.Contour_GenRotatedRectangle(minRect).Move((1, 5)).Zoom((50, 50))])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 4.410891532897949)
        self.assertAlmostEqual(minRect[0][1], 4.10891056060791)
        self.assertAlmostEqual(minRect[1][0], 12.238957405090332)
        self.assertAlmostEqual(minRect[1][1], 7.96029806137085)
        self.assertAlmostEqual(minRect[2], -95.71058654785156)

    def test_I18_MinAreaRect(self):
        plg = extradata.LoadTextPolygon('i18_nodes.txt')
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [plg.Move((5, 5)).Zoom((30, 30)), mvlab.Contour_GenRotatedRectangle(minRect).Move((5, 5)).Zoom((30, 30))])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 8.441173553466797)
        self.assertAlmostEqual(minRect[0][1], 10.235295295715332)
        self.assertAlmostEqual(minRect[1][0], 21.343135833740234)
        self.assertAlmostEqual(minRect[1][1], 15.764819145202637)
        self.assertAlmostEqual(minRect[2], 14.036243438720703)

    def test_Snake_MinAreaRect(self):
        plg = extradata.LoadTextPolygon('snake_nodes.txt')
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [plg.Move((1, 5)).Zoom((20, 20)), mvlab.Contour_GenRotatedRectangle(minRect).Move((1, 5)).Zoom((20, 20))])

        print(minRect)
        self.assertAlmostEqual(minRect[0][0], 25)
        self.assertAlmostEqual(minRect[0][1], 10)
        self.assertAlmostEqual(minRect[1][0], 50)
        self.assertAlmostEqual(minRect[1][1], 20)
        self.assertAlmostEqual(minRect[2], 0)

    def test_Random_Small_MinAreaRect(self):
        n = int(random.uniform(1, 30))
        points = []
        for i in range(0, n):
            points.append((random.uniform(150, 640), random.uniform(150, 480)))
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        eMinRect = mvlab.Contour_GenRotatedRectangle(((minRect[0][0], minRect[0][1]), (minRect[1][0]+1, minRect[1][1]+1), minRect[2]))
        for point in points:
            if not eMinRect.TestPoint(point):
                print(points)
            self.assertTrue(eMinRect.TestPoint(point))

    def test_Random_Large_MinAreaRect(self):
        n = int(random.uniform(1, 300))
        points = []
        for i in range(0, n):
            points.append((random.uniform(150, 640), random.uniform(150, 480)))
        plg = mvlab.Contour_GenPolygon(points)
        crosses = mvlab.Contour_GenCross(points, [(5, 5)]*n, [0]*n)
        minRect = plg.SmallestRectangle()
        extradata.SaveContours(self.id(), [crosses, plg.GetConvex(), mvlab.Contour_GenRotatedRectangle(minRect)])

        eMinRect = mvlab.Contour_GenRotatedRectangle(((minRect[0][0], minRect[0][1]), (minRect[1][0]+1, minRect[1][1]+1), minRect[2]))
        for point in points:
            if not eMinRect.TestPoint(point):
                print(points)
            self.assertTrue(eMinRect.TestPoint(point))

    def test_Performance_Digits(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'digits.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 50, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour().GetConvex()
        startTime = time.perf_counter()
        miniRects = c.GetSmallestRectangle()
        endTime = time.perf_counter()

        conts = [c]
        for miniRect in miniRects:
            conts.append(mvlab.Contour_GenRotatedRectangle(miniRect))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), conts)

    def test_Performance_Mista(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'idata', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        c = rgns.GetContour().GetConvex()
        startTime = time.perf_counter()
        miniRects = c.GetSmallestRectangle()
        endTime = time.perf_counter()

        conts = [c]
        for miniRect in miniRects:
            conts.append(mvlab.Contour_GenRotatedRectangle(miniRect))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), conts)

    def test_Performance_PCB(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'models', 'pcb_layout.png'), cv2.IMREAD_UNCHANGED)
        r, rgn = mvlab.Threshold(image, 0, 50)
        rgns = rgn.Connect()

        c = rgns.GetContour()
        startTime = time.perf_counter()
        miniRects = c.GetSmallestRectangle()
        endTime = time.perf_counter()

        conts = [c]
        for miniRect in miniRects:
            conts.append(mvlab.Contour_GenRotatedRectangle(miniRect))

        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveContours(self.id(), conts)

if __name__ == '__main__':
    unittest.main()
