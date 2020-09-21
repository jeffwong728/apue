import os
import sys
import cv2
import time
import mvlab
import numpy
import logging
import unittest
import extradata

class TestMatPersistence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    filename=os.path.join(os.environ['TEMP'], 'mvlab.log'),
                    filemode='a')

    @unittest.skip("Not needed anymore")
    def test_Save_Mat(self):
        mat = numpy.zeros((32, 24, 1), dtype = "uint8")
        db = mvlab.H5DB_Open(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'rdata', 'TestMatPersistence.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetMat("channels1", mat)
        self.assertEqual(r, 0)

        mat = numpy.zeros((64, 48, 3), dtype = "uint8")
        cv2.rectangle(mat, (10, 10), (30, 30), (0, 255, 0), 3)
        r = g.SetMat("channels3", mat)
        self.assertEqual(r, 0)

        mat = numpy.zeros((10, 64, 48), dtype = "uint8")
        r = g.SetMat("dim3", mat)
        self.assertEqual(r, 0)

        tmpl = mvlab.ContourTemplate_GenEmpty()
        r = g.SetContourTemplate("tmpl", tmpl)
        self.assertEqual(r, 0)

    def test_Load_Mat(self):
        mat = numpy.ones((32, 24), dtype = "uint8")
        db = mvlab.H5DB_Open(os.path.join(os.environ["SPAM_ROOT_DIR"], 'spam', 'unittest', 'rdata', 'TestMatPersistence.h5'))
        self.assertTrue(db.Valid())
        g = db.GetRoot()
        self.assertTrue(g.Valid())
        r = g.SetMat("channels1", mat)
        self.assertEqual(r, 0)
        mat1 = g.GetMat("channels1")
        self.assertTrue(numpy.array_equal(mat, mat1))

        mat = numpy.zeros((64, 48, 3), dtype = "uint8")
        cv2.rectangle(mat, (10, 10), (30, 30), (0, 255, 0), 3)
        r = g.SetMat("channels3", mat)
        self.assertEqual(r, 0)
        mat1 = g.GetMat("channels3")
        self.assertTrue(numpy.array_equal(mat, mat1))

        mat = numpy.ones((32, 24, 5), dtype = "int32")
        r = g.SetMat("dim3", mat)
        self.assertEqual(r, 0)
        mat1 = g.GetMat("dim3")
        self.assertTrue(numpy.array_equal(mat, mat1))

        mat = numpy.ones((32, 24, 5), dtype = "float32")
        r = g.SetMat("f3", mat)
        self.assertEqual(r, 0)
        mat1 = g.GetMat("f3")
        self.assertTrue(numpy.array_equal(mat, mat1))

        mat = numpy.ones((32, 24, 5, 3), dtype = "float64")
        r = g.SetMat("d4", mat)
        self.assertEqual(r, 0)
        mat1 = g.GetMat("d4")
        self.assertTrue(numpy.array_equal(mat, mat1))

if __name__ == '__main__':
    unittest.main()