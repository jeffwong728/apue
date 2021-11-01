import os
import sys
import numpy
import time
import argparse
import platform
import subprocess
import sysconfig

parser = argparse.ArgumentParser()
parser.add_argument('-D', '--Debug', action='store_true', help='Enable debug mode')
parser.add_argument('-R', '--Release', action='store_true', help='Enable release mode')

args = parser.parse_args()
if platform.system() == "Windows" and not sysconfig.get_platform() == "mingw":
    if args.Debug:
        os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'debug', 'bin'))
    else:
        os.add_dll_directory(os.path.join(os.environ["VCPKG_ROOT_DIR"], 'installed', 'x64-windows', 'bin'))

import cv2
import mvlab

def vtune_RLEDilation(times):
    se = mvlab.Region_GenStructuringElement('circle', 5)
    image = cv2.imread(os.path.join(os.environ["JANE_ROOT_DIR"], 'jane', 'test', 'data', 'images', 'board', 'board-01.png'), cv2.IMREAD_UNCHANGED)
    blue = image
    rgn = mvlab.Threshold(blue, 150, 255)
    opts = mvlab.Dict_GenEmpty()
    opts.SetString("Method", "RLEDilation")
    contr = rgn.GetContour()

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
    vtune_RLEDilation(5)