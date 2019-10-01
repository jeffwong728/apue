from __future__ import print_function
import sys
import pyautogui

try:
    while True:
        x, y = pyautogui.position()
        posStr = 'X: ' + str(x).rjust(4) + ' Y: ' + str(y).rjust(4)
        print(posStr, end='')
        print('\b'*len(posStr), end='')
except KeyboardInterrupt:
    print('\nDone')