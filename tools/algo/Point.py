import os
import sys
import math

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.label = ''

    def distance(self, other):
        return math.sqrt(math.pow(self.x-other.x, 2)+math.pow(self.y-other.y, 2))

    def __repr__(self):
        return 'Point'+str(self)

    def __str__(self):
        return '%s(%s, %s)' % (self.label, str(self.x), str(self.y))