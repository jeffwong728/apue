import os
import sys
import math
import bisect
import random
from cStringIO import StringIO

def PRINT_NEATLY(l, M):
    n = len(l)
    extras = []
    for i in range(0, n):
        extras.append([0]*n)
        extras[i][i] = M - l[i]
        for j in range(i+1, n):
            extras[i][j] = extras[i][j-1]-1-l[j]

    lc = []
    for i in range(0, n):
        lc.append([0]*n)
        for j in range(i, n):
            if extras[i][j]<0:
                lc[i][j] = sys.maxint
            elif (n-1)==j and extras[i][j]>0:
                lc[i][j] = 0
            else:
                lc[i][j] = math.pow(extras[i][j], 3)

    c = [-1]*n
    p = [-1]*n
    for j in range(0, n):
        c[j] = sys.maxint
        for i in range(0, j):
            s = c[i-1]+lc[i][j]
            if s<c[j]:
                c[j] = s
                p[j] = i

    return c,p

def GIVE_LINES(words, p, j):
    i = p[j]
    if i>0:
        k = GIVE_LINES(words, p, i-1)+1
    else:
        k = 1
    print " ".join(words[i:j+1])+"*"
    return k


def test():
    M=50
    para="""Kindle books are read across a wide variety of devices (e.g., Fire tablets and other manufacturers 
    smartphones and tablets) and a wide variety of screen dimensions. The 2013 Kindle Fire HD 8.9 has a resolution 
    of 1920 x 1200 pixels. Design the content to maintain this aspect ratio, if possible."""
    words = para.split()
    l=[len(w) for w in words]
    c, p = PRINT_NEATLY(l, M)
    print GIVE_LINES(words, p, len(words)-1)