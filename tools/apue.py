import mvlab
import spam
import time

def DoTest():
    s = spam.FindStation('station4')
    r = s.NewRect(500, 500, 480, 320)
    for i in range(0, 360):
        r.Rotate(1)
        time.sleep(0.05)