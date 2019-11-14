import os
import sys
import cv2
import mvlab
import numpy
import urllib

def init():
    global perfData
    global objPath
    perfData = dict()
    objPath = dict()

def SavePerformanceData(testId, secs):
    global perfData
    perfData.setdefault(testId, "{0:.3f}".format(secs*1000))

def SaveImage(testId, img):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)
    cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', img)

def SaveRegion(testId, rgn, sz=None):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)
    if not sz:
        sz = (0, 0)
    image = numpy.zeros((sz[0], sz[1], 4), numpy.uint8)
    r, image = rgn.Draw(image, (255, 255, 255, 255), (255, 255, 255, 255), 0, 0)

    if not r:
        cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)
        global objPath
        objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')