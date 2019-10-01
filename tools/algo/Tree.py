import os
import sys

class TreeNode:
    def __init__(self, p, l, r, v):
        self.p = p
        self.l = l
        self.r = r
        self.v = v

    def __repr__(self):
        return 'TreeNode(%s)' % self.v

    def __str__(self):
        return str(self.v)

class Tree:
    def __init__(self, r):
        self.r = r

    def appendLeft(self, parent, child):
        parent.l = child
        child.p = parent
        return child

    def appendRight(self, parent, child):
        parent.r = child
        child.p = parent
        return child

    def inOrder(self, r):
        if r:
            left = self.inOrder(r.l)
            left += str(r.v)
            left += " "
            left += self.inOrder(r.r)
            return left
        else:
            return ""

    def inOrderLoop(self):
        s = []
        o = ""
        root = self.r
        while root or s:
            if root:
                s.append(root)
                root = root.l
            else:
                o += str(s[-1].v)
                o += " "
                root = s.pop().r

        return o

    def __repr__(self):
        return 'Tree(%s)' % self.inOrder(self.r)

    def __str__(self):
        return self.inOrder(self.r)+"\n"+self.inOrderLoop()