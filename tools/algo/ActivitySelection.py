import os
import sys
import math
import bisect
import random
from cStringIO import StringIO

def DP_ACTIVITY_SELECTOR(s, f):
    s = s[:]
    f = f[:]
    s.insert(0, 0)
    f.insert(0, 0)
    s.append(sys.maxint)
    f.append(sys.maxint)

    S = []
    c = []
    d = []
    n = len(s)
    for i in range(0, n):
        S.append([0]*n)
        c.append([0]*n)
        d.append([-1]*n)

    for i in range(0, n):
        for j in range(i+1, n):
            S[i][j] = 0
            for k in range(i+1, j):
                if s[k]>=f[i] and f[k]<=s[j]:
                    S[i][j] += 1

    for l in range(2, n):
        for i in range(0, n-l):
            j = i+l
            if S[i][j]:
                c[i][j] = -1
                for k in range(i+1, j):
                    if s[k]>=f[i] and f[k]<=s[j]:
                        ck = c[i][k] + c[k][j] + 1
                        if ck > c[i][j]:
                            c[i][j] = ck
                            d[i][j] = k
            else:
                c[i][j] = 0

    return c, d

def SELECT_ACTIVITY(d, i, j):
    if d[i][j]<0:
        return []

    k = d[i][j]
    r = []
    r = r+SELECT_ACTIVITY(d, i, k)
    r.append(k)
    r = r + SELECT_ACTIVITY(d, k, j)

    return r

def test():
    s = [1, 3, 0, 5, 3, 5, 6, 8, 8, 2, 12]
    f = [4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]
    r, d = DP_ACTIVITY_SELECTOR(s, f)
    print SELECT_ACTIVITY(d, 0, len(s)+1)