import os
import sys
import cv2
import mvlab
import numpy

def init():
    global perfData
    global imgPath
    perfData = dict()
    imgPath = dict()

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
    image = numpy.zeros((sz[0], sz[1], 4), numpy.uint8)
    r, image = rgn.Draw(image, (255, 255, 255, 255), (255, 255, 255, 255), 0, 0)
    cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)