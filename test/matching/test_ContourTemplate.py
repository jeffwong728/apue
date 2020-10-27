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

class TestContourTemplate(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    @unittest.skip("This is a debug case")
    def test_Draw_Mista_ContourTemplate(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenRectangle((272, 110, 100,50))
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -5)
        copts.SetInt('AngleExtent', 10)
        copts.SetInt('PyramidLevel', 4)
        copts.SetInt('LowContrast', 10)
        copts.SetInt('HighContrast', 15)
        copts.SetRegion('ROI', rgn)
        tmpl = mvlab.ContourTemplate_GenTemplate(blue, rgn, copts)

        outputRoot = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "contour_template")
        max_trys = 10
        while os.path.exists(outputRoot) and max_trys > 0:
            shutil.rmtree(outputRoot, ignore_errors=True)
            max_trys -= 1
            time.sleep(1.0)
        if not os.path.exists(outputRoot):
            os.makedirs(outputRoot)

        dopts = mvlab.Dict_GenEmpty()
        dopts.SetString('DebugFullPath', outputRoot)
        #r, i = tmpl.Draw(blue, dopts)
        dopts.SetString('FileFormat', "binary")
        dopts.SetString('Policy', "backup")
        r = tmpl.Save(os.path.join(outputRoot, "scrach.bin"), dopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())
        db = mvlab.H5DB_Open(os.path.join(outputRoot, 'data.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetContourTemplate("scrach", tmpl)
        self.assertEqual(0, r, g.GetErrorStatus())

    def test_Save_Scrach_ContourTemplate(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenRectangle((338, 213, 89, 97))
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -3)
        copts.SetInt('AngleExtent', 6)
        copts.SetInt('PyramidLevel', 4)
        copts.SetInt('LowContrast', 10)
        copts.SetInt('HighContrast', 15)
        copts.SetRegion('ROI', rgn)
        tmpl = mvlab.ContourTemplate_GenTemplate(blue, rgn, copts)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
        pathComps = self.id().split(sep='.')
        savePath = os.path.join(baseDir, *pathComps[0:-2])
        os.makedirs(savePath, exist_ok=True)

        sopts = mvlab.Dict_GenEmpty()
        sopts.SetString('FileFormat', "text")
        sopts.SetString('Policy', "overwrite")
        r = tmpl.Save(os.path.join(savePath, "scrach_contour_template.txt.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "xml")
        r = tmpl.Save(os.path.join(savePath, "scrach_contour_template.xml.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        sopts.SetString('FileFormat', "binary")
        r = tmpl.Save(os.path.join(savePath, "scrach_contour_template.bin.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

    def test_Save_Gear_ContourTemplate(self):
        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'gear', 'Template.jpg'))
        blue, green, red = cv2.split(image)

        rgn = mvlab.Region_GenCircle((300, 310), 100)
        copts = mvlab.Dict_GenEmpty()
        copts.SetInt('AngleStart', -10)
        copts.SetInt('AngleExtent', 20)
        copts.SetInt('PyramidLevel', 5)
        copts.SetInt('LowContrast', 7)
        copts.SetInt('HighContrast', 15)
        tmpl = mvlab.ContourTemplate_GenTemplate(blue, rgn, copts)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
        pathComps = self.id().split(sep='.')
        savePath = os.path.join(baseDir, *pathComps[0:-2])
        os.makedirs(savePath, exist_ok=True)

        sopts = mvlab.Dict_GenEmpty()
        sopts.SetString('FileFormat', "text")
        sopts.SetString('Policy', "overwrite")
        r = tmpl.Save(os.path.join(savePath, "gear_contour_template.txt.xz"), sopts)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

    @unittest.skip("This is a debug case")
    def test_Load_ContourTemplate(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "scrach.txt")
        tmpl = mvlab.ContourTemplate_GenEmpty()
        r = tmpl.Load(fileName)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "scrach.xml")
        tmpl = mvlab.ContourTemplate_GenEmpty()
        r = tmpl.Load(fileName)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "scrach.bin")
        tmpl = mvlab.ContourTemplate_GenEmpty()
        r = tmpl.Load(fileName)
        self.assertEqual(0, r, tmpl.GetErrorStatus())

    def test_Match_Scrach_ContourTemplate(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "template", "scrach_contour_template.txt.xz")
        tmpl = mvlab.ContourTemplate_Load(fileName)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'scrach.png'))
        blue, green, red = cv2.split(image)
        height, width, channels = image.shape

        opts = mvlab.Dict_GenEmpty()
        opts.SetReal32("MinScore", 0.8)
        opts.SetReal32("Greediness", 0.5)
        opts.SetInt("MinContrast", 10)
        opts.SetInt("TouchBorder", 0)
        opts.SetRegion("SearchRegion", mvlab.Region_GenRectangle((0, 0, width, height)))

        startTime = time.perf_counter()
        mr = tmpl.Search(blue, opts)
        endTime = time.perf_counter()
        self.assertIsNotNone(mr)
        self.assertEqual(1, mr.Count())
        mr.Draw(image)

        print("Position: ({0[0]:.3f}, {0[1]:.3f}, Angle: {1:.3f}, Score: {2:.3f})".format(mr.GetPosition(), mr.GetAngle(), mr.GetScore()))
        extradata.SavePerformanceData(self.id(), endTime-startTime)
        extradata.SaveImage(self.id(), image)

    def test_Match_Gear_ContourTemplate(self):
        fileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', "data", "model", "template", "gear_contour_template.txt.xz")
        tmpl = mvlab.ContourTemplate_Load(fileName)
        self.assertFalse(tmpl.Empty(), tmpl.GetErrorStatus())

        opts = mvlab.Dict_GenEmpty()
        opts.SetReal32("MinScore", 0.5)
        opts.SetReal32("Greediness", 0.5)
        opts.SetInt("MinContrast", 8)
        opts.SetInt("TouchBorder", 1)

        dopts = mvlab.Dict_GenEmpty()
        dopts.SetReal32("ArrowThickness", 2.5)
        dopts.SetScalar("ArrowColor", (255, 0, 255, 255))
        dopts.SetScalar("RegionColor", (255, 0, 255, 255))

        totalTime = 0
        for imgName in ['Search1.jpg', 'Search2.jpg', 'Search3.jpg']:
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
            mr.Draw(image, dopts)
            extradata.AddImage(self.id(), image, imgName)

        print("Position: ({0[0]:.3f}, {0[1]:.3f}, Angle: {1:.3f}, Score: {2:.3f})".format(mr.GetPosition(), mr.GetAngle(), mr.GetScore()))
        extradata.SavePerformanceData(self.id(), totalTime/3)

if __name__ == '__main__':
    unittest.main()