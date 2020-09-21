import os
import sys
import cv2
import mvlab
import numpy
import urllib

def init():
    global perfData
    global objPath
    global objPaths
    global colors
    global nextcolor
    perfData = dict()
    objPaths = dict()
    objPath = dict()
    colors = [(0xFF, 0x00, 0x00, 0xFF), (0x00, 0xFF, 0x00, 0xFF), (0x00, 0x00, 0xFF, 0xFF)]
    colors.extend([(0xFF, 0x00, 0xFF, 0xFF), (0xFF, 0xFF, 0x00, 0xFF), (0x00, 0xFF, 0xFF, 0xFF)])
    colors.extend([(0xda, 0xa5, 0x20, 0xFF), (0xff, 0xc8, 0xcb, 0xFF), (0xff, 0xa5, 0x00, 0xFF)])
    colors.extend([(0xff, 0x7f, 0x50, 0xFF), (0x40, 0xE0, 0xD0, 0xFF), (0x80, 0x00, 0x80, 0xFF)])
    colors.extend([(0x4B, 0x00, 0x82, 0xFF), (0xEE, 0x82, 0xEE, 0xFF), (0x80, 0x00, 0x00, 0xFF)])
    colors.extend([(0x00, 0x00, 0x80, 0xFF), (0xFF, 0xFf, 0xFF, 0xFF), (0x80, 0x80, 0x80, 0xFF)])
    colors.extend([(0xD3, 0xD3, 0xD3, 0xFF), (0xA9, 0xA9, 0xA9, 0xFF), (0x2F, 0x4F, 0x4F, 0xFF)])
    colors.extend([(0x00, 0xFF, 0x7F, 0xFF), (0xFF, 0x63, 0x47, 0xFF), (0xff, 0xff, 0xff, 0xFF)])
    nextcolor = 0

def GetNextColor():
    global colors
    global nextcolor
    color = colors[nextcolor]
    nextcolor = nextcolor + 1
    if not nextcolor < len(colors):
        nextcolor = 0
    return color

def RectUnion(a,b):
    x = min(a[0], b[0])
    y = min(a[1], b[1])
    w = max(a[0]+a[2], b[0]+b[2]) - x
    h = max(a[1]+a[3], b[1]+b[3]) - y
    return (x, y, w, h)

def RectIntersection(a,b):
    x = max(a[0], b[0])
    y = max(a[1], b[1])
    w = min(a[0]+a[2], b[0]+b[2]) - x
    h = min(a[1]+a[3], b[1]+b[3]) - y
    if w<0 or h<0: return (0, 0, 0, 0)
    return (x, y, w, h)

def SavePerformanceData(testId, secs):
    global perfData
    perfData.setdefault(testId, "{0:.3f}".format(secs*1000))

def SaveImage(testId, img):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)
    cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', img)
    global objPath
    objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')

def AddImage(testId, img, name):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:])
    os.makedirs(imgPath, exist_ok=True)
    cv2.imwrite(os.path.join(baseDir, *imgPathComps, name), img)
    global objPaths
    paths = objPaths.setdefault(testId, [])
    imgPathComps.append(name)
    paths.append('/'.join(imgPathComps))

def SaveRegion(testId, rgn, sz=None):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)
    shape = None
    if not sz:
        sz = (0, 0, 0, 0)
        sz = RectUnion(sz, rgn.BoundingBox())
        shape = (sz[3]+10, sz[2]+10, 4)
    else:
        shape = (sz[0]+10, sz[1]+10, 4)
    image = numpy.zeros(shape, numpy.uint8)
    r, image = rgn.Draw(image, (255, 255, 255, 128))

    if not r:
        cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)
        global objPath
        objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')

def DrawRegion(testId, rgn, image):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    r, image = rgn.Draw(image, (255, 0, 0, 128))

    if not r:
        cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)
        global objPath
        objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')

def SaveRegions(testId, rgns, sz=None):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)

    shape = None
    if not sz:
        sz = (0, 0, 0, 0)
        for rgn in rgns:
            sz = RectUnion(sz, rgn.BoundingBox())
        shape = (sz[3]+10, sz[2]+10, 4)
    else:
        shape = (sz[0]+10, sz[1]+10, 4)

    image = numpy.zeros(shape, numpy.uint8)
    for rgn in rgns:
        r, image = rgn.Draw(image, GetNextColor())

    if not r:
        cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)
        global objPath
        objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')

def SaveContours(testId, cturs, sz=None):
    baseDir = os.path.join(os.environ['SPAM_ROOT_DIR'], 'reports')
    imgPathComps = testId.split(sep='.')
    del imgPathComps[-2]
    imgPath = os.path.join(baseDir, *imgPathComps[0:-1])
    os.makedirs(imgPath, exist_ok=True)

    shape = None
    if not sz:
        sz = (0, 0, 0, 0)
        for ctur in cturs:
            for box in ctur.GetBoundingBox():
                sz = RectUnion(sz, box)
        shape = (sz[3]+10, sz[2]+10, 4)
    else:
        shape = (sz[0], sz[1], 4)

    image = numpy.zeros(shape, numpy.uint8)
    for ctur in cturs:
        r, image = ctur.Draw(image, GetNextColor(), 1.5, 0)

    if not r:
        cv2.imwrite(os.path.join(baseDir, *imgPathComps) + '.png', image)
        global objPath
        objPath.setdefault(testId, '/'.join(imgPathComps)+'.png')

def LoadTextPolygon(fileName):
    fullFileName = os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'plg', fileName)
    points = []
    with open(fullFileName, 'rt') as f:
        for ln in f:
            wds = ln.split()
            if 2==len(wds):
                try:
                    x = float(wds[0])
                    y = float(wds[1])
                    points.append((x, y))
                except ValueError:
                    pass
    return mvlab.Contour_GenPolygon(points)