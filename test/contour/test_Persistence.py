import os
import sys
import cv2
import mvlab
import unittest
import numpy
import random
import time
import logging
import extradata

class TestContourPersistence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Save_Contour(self):
        baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
        pathComps = self.id().split(sep='.')
        savePath = os.path.join(baseDir, *pathComps[0:-2])
        os.makedirs(savePath, exist_ok=True)

        opts = mvlab.Dict_GenEmpty()
        opts.SetString('Policy', 'overwrite')
        opts.SetString('FileFormat', "text")
        fileName = os.path.join(savePath, "rectangle_contour.txt.xz")
        cont = mvlab.Contour_GenRectangle((0, 0, 10, 10))
        r = cont.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "xml")
        fileName = os.path.join(savePath, "rectangle_contour.xml.xz")
        r = cont.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "binary")
        fileName = os.path.join(savePath, "rectangle_contour.bin.xz")
        r = cont.Save(fileName, opts)
        self.assertEqual(0, r)

        points = []
        sizes = []
        angles = []
        random.seed()
        for i in range(0, 3):
            points.append((random.uniform(0, 640), random.uniform(0, 480)))
            sizes.append((10, 10))
            angles.append(random.uniform(0, 360))
        cont = mvlab.Contour_GenCross(points, sizes, angles)
        opts.SetString('FileFormat', "xml")
        fileName = os.path.join(savePath, "crosses.xml.xz")
        r = cont.Save(fileName, opts)
        self.assertEqual(0, r)

    def test_Load_Contour(self):
        loadPath = os.path.join(os.environ['SPAM_ROOT_DIR'], 'test', "data", "model", "contour")
        cont = mvlab.Contour_Load(os.path.join(loadPath, "rectangle_contour.xml.xz"))
        self.assertIsNotNone(cont)
        self.assertEqual(100, cont.Area(), cont.GetErrorStatus())

        cont = mvlab.Contour_Load(os.path.join(loadPath, "rectangle_contour.txt.xz"))
        self.assertIsNotNone(cont)
        self.assertEqual(100, cont.Area(), cont.GetErrorStatus())

        cont = mvlab.Contour_Load(os.path.join(loadPath, "rectangle_contour.bin.xz"))
        self.assertIsNotNone(cont)
        self.assertEqual(100, cont.Area(), cont.GetErrorStatus())

        cont = mvlab.Contour_Load(os.path.join(loadPath, "crosses.xml.xz"))
        self.assertIsNotNone(cont)
        self.assertEqual(3, cont.Count(), cont.GetErrorStatus())

    def test_SaveDB_Rectangle_Contour(self):
        outputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach")
        cont = mvlab.Contour_GenRectangle((0, 0, 10, 10))

        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetContour("rectangle_contour", cont)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_SaveDB_Mista_Contours(self):
        outputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach")
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()
        conts = rgns.GetContour()

        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetContour("mista_contours", conts)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_LoadDB_Rectangle_Contour(self):
        inputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        cont = g.GetContour("rectangle_contour")
        self.assertIsNotNone(cont, g.GetErrorStatus())
        self.assertEqual(100, cont.Area(), cont.GetErrorStatus())

    def test_LoadDB_Mista_Contours(self):
        inputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        conts = g.GetContour("mista_contours")
        self.assertIsNotNone(conts, g.GetErrorStatus())
        self.assertEqual(941, conts.Count(), conts.GetErrorStatus())

if __name__ == '__main__':
    unittest.main()