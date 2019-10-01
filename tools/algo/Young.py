import os
import sys
class Young:
    def __init__(self, m, n, *elems):
        if m<1:
            raise OverflowError
        if n<1:
            raise OverflowError

        s = len(elems)
        if s>m*n:
            raise OverflowError

        for e in elems:
            if not e<sys.maxint:
                raise OverflowError

        self.m = m;
        self.n = n;

        self.data = []
        for r in range(0, m):
            self.data.append([sys.maxint]*n)

        for e in elems:
            self.insert(e)

    def expand(self):
        self.m += 1
        self.n += 1
        self.data.append([sys.maxint]*self.n)
        for r in self.data[0:-1]:
            r.append(sys.maxint)

    def normalize(self, i, j):
        largesti = i
        largestj = j

        if i>0 and self.data[i-1][j] > self.data[i][j]:
            largesti = i-1
            largestj = j

        if j>0 and self.data[i][j-1] > self.data[largesti][largestj]:
            largesti = i
            largestj = j-1

        if largesti != i or largestj != j:
            self.data[largesti][largestj], self.data[i][j] = self.data[i][j], self.data[largesti][largestj]
            self.normalize(largesti, largestj)

    def insert(self, e):
        if self.data[self.m-1][self.n-1]<sys.maxint:
            self.expand()

        self.data[self.m-1][self.n-1] = e
        self.normalize(self.m-1, self.n-1)
    
    def __repr__(self):
        return 'Young(%s)' % self.data

    def __str__(self):
        s=""
        for i in range(0, self.m):
            for j in range(0, self.n):
                s += '%10d | ' % self.data[i][j]
            s += os.linesep
        return s