import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata

class TestTemplatePersistence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_SaveDB_Gear_ContourTemplate(self):
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'gear', 'Template1.jpg'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenCircle((300, 310), 100)
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -100)
        copts.SetInt('AngleExtent', 200)
        copts.SetInt('PyramidLevel', 5)
        copts.SetInt('LowContrast', 7)
        copts.SetInt('HighContrast', 15)
        tmpl = mvlab.ContourTemplate_GenTemplate(blue, rgn, copts)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        outputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "scrach")
        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        r = g.SetContourTemplate("gear_contour_template", tmpl)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_SaveDB_Gear_SADTemplate(self):
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'gear', 'Template1.jpg'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenCircle((300, 310), 100)
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -20)
        copts.SetInt('AngleExtent', 40)
        copts.SetInt('PyramidLevel', 5)
        copts.SetString("MatchMode", "sad")
        tmpl = mvlab.PixelTemplate_GenTemplate(blue, rgn, copts)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        outputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "scrach")
        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        r = g.SetPixelTemplate("gear_sad_template", tmpl)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_SaveDB_Gear_NCCTemplate(self):
        image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'test', 'data', 'images', 'gear', 'Template1.jpg'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenCircle((300, 310), 100)
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -20)
        copts.SetInt('AngleExtent', 40)
        copts.SetInt('PyramidLevel', 5)
        copts.SetString("MatchMode", "ncc")
        tmpl = mvlab.PixelTemplate_GenTemplate(blue, rgn, copts)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        outputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "scrach")
        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        r = g.SetPixelTemplate("gear_ncc_template", tmpl)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_LoadDB_Gear_ContourTemplate(self):
        inputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        tmpl = g.GetContourTemplate("gear_contour_template")
        self.assertIsNotNone(tmpl, g.GetErrorStatus())
        self.assertEqual(5, tmpl.GetPyramidLevel(), tmpl.GetErrorStatus())

    def test_LoadDB_Gear_SADTemplate(self):
        inputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        tmpl = g.GetPixelTemplate("gear_sad_template")
        self.assertIsNotNone(tmpl, g.GetErrorStatus())
        self.assertEqual(5, tmpl.GetPyramidLevel(), tmpl.GetErrorStatus())

    def test_LoadDB_Gear_NCCTemplate(self):
        inputRoot = os.path.join(os.environ["JANE_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        tmpl = g.GetPixelTemplate("gear_ncc_template")
        self.assertIsNotNone(tmpl, g.GetErrorStatus())
        self.assertEqual(5, tmpl.GetPyramidLevel(), tmpl.GetErrorStatus())

if __name__ == '__main__':
    unittest.main()