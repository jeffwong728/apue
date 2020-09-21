import os
import sys
import cv2
import mvlab
import numpy
import time

def vtune_RLEDilation(times):
    se = mvlab.Region_GenStructuringElement('circle', 5)
    image = cv2.imread(os.path.join(os.environ["SPAM_ROOT_DIR"], 'test', 'data', 'images', 'mista.png'), cv2.IMREAD_UNCHANGED)
    blue, green, red = cv2.split(image)
    r, rgn = mvlab.Threshold(blue, 150, 255)
    opts = mvlab.Dict_GenEmpty()
    opts.SetString("Method", "RLEDilation")

    startTime = time.perf_counter()
    for t in range(0, times):
        rlergn = rgn.Dilation(se, opts)
    endTime = time.perf_counter()
    print('RLEDilation: {0:.3f}ms'.format((endTime-startTime)*1000/times))

if "__main__"==__name__:
    print("***************************************************************")
    print("****** Please attach to python.exe to do VTune analysis. ******")
    print("***************************************************************")
    os.system('pause')
    vtune_RLEDilation(500)