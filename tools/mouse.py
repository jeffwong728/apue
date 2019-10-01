import sys
import pyautogui

while True:
    pyautogui.moveRel(5, 0, duration=1)
    pyautogui.moveRel(0, 5, duration=1)
    pyautogui.moveRel(-5, 0, duration=1)
    pyautogui.moveRel(0, -5, duration=1)