import os
import sys
import cv2
import mvlab
import unittest
import numpy
import time
import logging
import extradata
import shutil

class TestPixelTemplate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    def test_Save_Scrach_SADTemplate(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenRectangle((272, 110, 100, 50))
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -20)
        copts.SetInt('AngleExtent', 40)
        copts.SetInt('PyramidLevel', 4)
        copts.SetString("MatchMode", "sad")
        tmpl = mvlab.PixelTemplate_GenTemplate(blue, rgn, copts)

        baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
        pathComps = self.id().split(sep='.')
        savePath = os.path.join(baseDir, *pathComps[0:-2])
        os.makedirs(savePath, exist_ok=True)

        sopts = mvlab.Dict_GenEmpty()
        sopts.SetString('FileFormat', "text")
        sopts.SetString('Policy', "overwrite")
        r = tmpl.Save(os.path.join(savePath, "scrach_sad_template.txt.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "xml")
        r = tmpl.Save(os.path.join(savePath, "scrach_sad_template.xml.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "binary")
        r = tmpl.Save(os.path.join(savePath, "scrach_sad_template.bin.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

    def test_Save_Scrach_NCCTemplate(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenRectangle((272, 110, 100, 50))
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -20)
        copts.SetInt('AngleExtent', 40)
        copts.SetInt('PyramidLevel', 4)
        copts.SetString("MatchMode", "ncc")
        tmpl = mvlab.PixelTemplate_GenTemplate(blue, rgn, copts)

        baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
        pathComps = self.id().split(sep='.')
        savePath = os.path.join(baseDir, *pathComps[0:-2])
        os.makedirs(savePath, exist_ok=True)

        sopts = mvlab.Dict_GenEmpty()
        sopts.SetString('FileFormat', "text")
        sopts.SetString('Policy', "overwrite")
        r = tmpl.Save(os.path.join(savePath, "scrach_ncc_template.txt.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "xml")
        r = tmpl.Save(os.path.join(savePath, "scrach_ncc_template.xml.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "binary")
        r = tmpl.Save(os.path.join(savePath, "scrach_ncc_template.bin.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

    def test_Match_Gear_NCCTemplate(self):
        inputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model")
        db = mvlab.H5DB_Open(os.path.join(inputRoot, 'database.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        g = g.GetGroup('template')
        tmpl = g.GetPixelTemplate("gear_ncc_template")

        opts = mvlab.Dict_GenEmpty()
        opts.SetReal32("MinScore", 0.8)

        totalTime = 0
        for imgName in ['Template1.jpg', 'Template2.jpg']:
            image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'gear', imgName))
            blue, green, red = cv2.split(image)
            height, width, channels = image.shape
            opts.SetRegion("SearchRegion", mvlab.Region_GenRectangle((0, 0, width, height)))

            startTime = time.perf_counter()
            mr = tmpl.Search(blue, opts)
            endTime = time.perf_counter()
            totalTime += (endTime-startTime)
            self.assertIsNotNone(mr)
            self.assertEqual(1, mr.Count())
            mr.Draw(image)
            extradata.AddImage(self.id(), image, imgName)

        print("Position: ({0[0]:.3f}, {0[1]:.3f}, Angle: {1:.3f}, Score: {2:.3f})".format(mr.GetPosition(), mr.GetAngle(), mr.GetScore()))
        extradata.SavePerformanceData(self.id(), totalTime/2)

if __name__ == '__main__':
    unittest.main()