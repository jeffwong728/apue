import os
import sys
import Point

class BitonicTour:
    def __init__(self, points):
        self.points = points
        self.points.sort(lambda point1, point2: point1.x<point2.x)
        n = len(self.points)
        for (p, r) in zip(self.points, range(1, n+1)):
            p.label = "p"+str(r)

        self.h =[]
        for i in range(0, n):
            self.h.append([0]*n)

        self.r =[]
        for i in range(0, n):
            self.r.append([0]*n)

        self.h[0][1] = self.points[0].distance(self.points[1])

        for j in range(2, n):
            for i in range(0, j-1):
                self.h[i][j] = self.h[i][j-1]+self.points[j-1].distance(self.points[j])
                self.r[i][j] = j-1
            self.h[j-1][j]=sys.maxint
            for k in range(0, j-1):
                h = self.h[k][j-1] + self.points[k].distance(self.points[j])
                if h<self.h[j-1][j]:
                    self.h[j-1][j] = h
                    self.r[j-1][j] = k

        self.h[n-1][n-1] = self.h[n-2][n-1] + self.points[n-2].distance(self.points[n-1])
        self.PRINT_TOUR()

    def PRINT_TOUR(self):
        n = len(self.points)
        print "p"+str(n)
        print "p"+str(n-1)
        k=self.r[n-2][n-1]
        self.PRINT_PATH(k, n-2)
        print "p"+str(k+1)

    def PRINT_PATH(self, i, j):
        if i<j:
            k=self.r[i][j]
            print "p"+str(k+1)
            if k>0:
                self.PRINT_PATH(i, k)
        else:
            k=self.r[j][i]
            if k>0:
                self.PRINT_PATH(k, j)
                print "p"+str(k+1)

    def __repr__(self):
        return 'BitonicTour'+str(self)

    def __str__(self):
        s="["
        if self.points:
            s += str(self.points[0])
            for p in self.points[1:]:
                s += ", "
                s += str(p)
        s += "]"
        return s