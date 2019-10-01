import os
import sys

def EUCLID(a, b):
    if not b:
        return a
    else:
        return EUCLID(b, a%b)

def EUCLID_ITERATIVE(a, b):
    while b:
        t=a
        a=b
        b=t%b
    return a

def EXTENDED_EUCLID(a, b):
    if not b:
        return (a, 1, 0)
    else:
        d, x, y = EXTENDED_EUCLID(b, a%b)
        return (d, y, x-(a/b)*y)

def test():
    print "EUCLID(50, 0)="+str(EUCLID(50, 0))
    print "EUCLID(50, 25)="+str(EUCLID(50, 25))
    print "EUCLID(25, 50)="+str(EUCLID(25, 50))
    print "EUCLID(0, 50)="+str(EUCLID(0, 50))
    print "EUCLID(1, 50)="+str(EUCLID(1, 50))
    print "EUCLID(15, 8)="+str(EUCLID(15, 8))
    print "EUCLID(99, 78)="+str(EUCLID(99, 78))
    print ""
    print "EUCLID(50, 0)="+str(EUCLID_ITERATIVE(50, 0))
    print "EUCLID(50, 25)="+str(EUCLID_ITERATIVE(50, 25))
    print "EUCLID(25, 50)="+str(EUCLID_ITERATIVE(25, 50))
    print "EUCLID(0, 50)="+str(EUCLID_ITERATIVE(0, 50))
    print "EUCLID(1, 50)="+str(EUCLID_ITERATIVE(1, 50))
    print "EUCLID(15, 8)="+str(EUCLID_ITERATIVE(15, 8))
    print "EUCLID(99, 78)="+str(EUCLID_ITERATIVE(99, 78))
    print ""
    print "EXTENDED_EUCLID(50, 0)="+str(EXTENDED_EUCLID(50, 0))
    print "EXTENDED_EUCLID(50, 25)="+str(EXTENDED_EUCLID(50, 25))
    print "EXTENDED_EUCLID(25, 50)="+str(EXTENDED_EUCLID(25, 50))
    print "EXTENDED_EUCLID(0, 50)="+str(EXTENDED_EUCLID(0, 50))
    print "EXTENDED_EUCLID(1, 50)="+str(EXTENDED_EUCLID(1, 50))
    print "EXTENDED_EUCLID(15, 8)="+str(EXTENDED_EUCLID(15, 8))
    print "EXTENDED_EUCLID(99, 78)="+str(EXTENDED_EUCLID(99, 78))
    print "EXTENDED_EUCLID(899, 493)="+str(EXTENDED_EUCLID(899, 493))