import os
import sys
import math
import bisect
import random
from cStringIO import StringIO

opCopy = {"op":0, "cost":1}
opReplace = {"op":1, "cost":2}
opDelete = {"op":2, "cost":2}
opInsert = {"op":3, "cost":2}
opTwiddle = {"op":4, "cost":3}
opKill = {"op":5, "cost":4, "index":0}

def EDIT_DISTANCE(x, y):
    x = x[:]
    y = y[:]
    x.insert(0, 0)
    y.insert(0, 0)
    m = len(x)
    n = len(y)

    c = []
    op = []
    for i in range(0, m):
        c.append([0]*n)
        op.append([0]*n)
        c[i][0] = i*opDelete["cost"]
    for j in range(0, n):
        c[0][j] = j*opInsert["cost"]

    for i in range(1, m):
        for j in range(1, n):
            c[i][j] = sys.maxint
            if x[i]==y[j]:
                c[i][j] = c[i-1][j-1] + opCopy["cost"]
                op[i][j] = opCopy

            if x[i]!=y[j] and c[i-1][j-1]+opReplace["cost"]<c[i][j]:
                c[i][j] = c[i-1][j-1]+opReplace["cost"]
                op[i][j] = opReplace

            if i>=2 and j>=2 and x[i]==y[j-1] and x[i-1]==y[j] and c[i-2][j-2]+opTwiddle["cost"]<c[i][j]:
                c[i][j] = c[i-2][j-2]+opTwiddle["cost"]
                op[i][j] = opTwiddle

            if c[i-1][j]+opDelete["cost"]<c[i][j]:
                c[i][j] = c[i-1][j]+opDelete["cost"]
                op[i][j] = opDelete

            if c[i][j-1]+opInsert["cost"]<c[i][j]:
                c[i][j] = c[i][j-1]+opInsert["cost"]
                op[i][j] = opInsert

    for i in range(0, m-1):
        if c[i][n-1]+opKill["cost"]<c[m-1][n-1]:
            c[m-1][n-1] = c[i][n-1]+opKill["cost"]
            opKillI = opKill.copy()
            opKillI["index"] = i
            op[m-1][n-1] = opKillI

    return c, op

def OP_SEQUENCE(op, i, j, x, y):
    if i==0 and j==0:
        return
    opStr = ""
    if op[i][j]["op"]==opCopy["op"]:
        ii = i-1
        jj = j-1
        opStr = "copy   "
    elif op[i][j]["op"]==opReplace["op"]:
        ii = i-1
        jj = j-1
        opStr = "replace"
    elif op[i][j]["op"]==opTwiddle["op"]:
        ii = i-2
        jj = j-2
        opStr = "twiddle"
    elif op[i][j]["op"]==opDelete["op"]:
        ii = i-1
        jj = j
        opStr = "delete "
    elif op[i][j]["op"]==opInsert["op"]:
        ii = i
        jj = j-1
        opStr = "insert "
    else:
        ii = op[i][j]["index"]
        jj = j
        opStr = "kill"
    OP_SEQUENCE(op, ii, jj, x, y)
    print opStr, "".join(x[0:i]), "".join(y[0:j])

def test():
    x = list("GTCGCTCTTTCGGTT")
    y = list("GTGGCTCTCTCGGTT")
    c, op = EDIT_DISTANCE(x, y)
    OP_SEQUENCE(op, len(x), len(y), x, y)