import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestRegionPersistence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pass

    def test_Save_Empty_Region(self):
        opts = mvlab.Dict_GenEmpty()
        opts.SetString('Policy', 'overwrite')
        opts.SetString('FileFormat', "text")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "empty_region.txt.xz")
        rgn = mvlab.Region_GenEmpty()
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "xml")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "empty_region.xml.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "binary")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "empty_region.bin.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

    def test_Save_Rectangle_Region(self):
        opts = mvlab.Dict_GenEmpty()
        opts.SetString('Policy', 'overwrite')
        opts.SetString('FileFormat', "text")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "rectangle_region.txt.xz")
        rgn = mvlab.Region_GenRectangle((0, 0, 10, 10))
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "xml")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "rectangle_region.xml.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "binary")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "rectangle_region.bin.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

    def test_Save_Mista_Region(self):
        opts = mvlab.Dict_GenEmpty()
        opts.SetString('Policy', 'overwrite')
        opts.SetString('FileFormat', "text")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "mista_region.txt.xz")
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "xml")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "mista_region.xml.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

        opts.SetString('FileFormat', "binary")
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach", "mista_region.bin.xz")
        r = rgn.Save(fileName, opts)
        self.assertEqual(0, r)

    def test_Load_Empty_Region(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "empty_region.txt.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "empty_region.xml.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "empty_region.bin.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

    def test_Load_Rectangle_Region(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "rectangle_region.txt.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(100, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "rectangle_region.xml.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(100, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "rectangle_region.bin.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(100, rgn.Area(), rgn.GetErrorStatus())

    def test_Load_Mista_Region(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "mista_region.txt.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(1932245, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "mista_region.xml.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(1932245, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "mista_region.bin.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(1932245, rgn.Area(), rgn.GetErrorStatus())

        extradata.SaveRegion(self.id(), rgn.Shrink((0.25, 0.25)))

    def test_Load_Damaged_Region(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "empty_file.txt.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "damaged_region.txt.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "region", "damaged_region.xml.xz")
        rgn = mvlab.Region_Load(fileName)
        self.assertEqual(0, rgn.Area(), rgn.GetErrorStatus())

    def test_SaveDB_Rectangle_Region(self):
        outputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach")
        rgn = mvlab.Region_GenRectangle((0, 0, 10, 10))

        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetRegion("rectangle_region", rgn)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_SaveDB_Mista_Regions(self):
        outputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "scrach")
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'))
        blue, green, red = cv2.split(image)
        r, rgn = mvlab.Threshold(blue, 150, 255)
        rgns = rgn.Connect()

        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetRegion("mista_regions", rgns)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_LoadDB_Rectangle_Region(self):
        inputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        rgn = g.GetRegion("rectangle_region")
        self.assertIsNotNone(rgn, g.GetErrorStatus())
        self.assertEqual(100, rgn.Area(), rgn.GetErrorStatus())

    def test_LoadDB_Mista_Regions(self):
        inputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        rgns = g.GetRegion("mista_regions")
        self.assertIsNotNone(rgns, g.GetErrorStatus())
        self.assertEqual(941, rgns.Count(), rgns.GetErrorStatus())

if __name__ == '__main__':
    unittest.main()
