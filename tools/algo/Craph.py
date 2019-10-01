import os
import sys
import math
import collections

WHITE = 0
GRAY  = 1
BLACK = 2

class V:
    def __init__(self, k, d=None):
        self.k    = k
        self.data = d
        self.d    = -1
        self.f    = -1
        self.e    = 0

    def __cmp__(self, other):
        if self.k < other.k:
            return -1
        elif self.k > other.k:
            return 1
        else:
            return 

    def __hash__(self):
        return hash(self.k)

    def __str__(self):
        return 'V(%s, %d, %d)' % (str(self.k), self.d, self.f)

class E:
    def __init__(self, u, v):
        self.u = u
        self.v = v

    def __str__(self):
        return 'E(%s->%s)' % (str(self.u), str(self.v))

class G:
    def __init__(self, directed):
        self.directed = directed
        self.edges = collections.OrderedDict()
        self.vertices = set()

    def reset(self):
        for v in self.vertices:
            v.pi    = None
            v.d     = -1
            v.f     = -1
            v.e     = 0
            v.color = WHITE

    def addVertex(self, v):
        self.vertices.add(v)

    def addEdge(self, e):
        self.vertices.add(e.u)
        self.vertices.add(e.v)
        self.addSinglesidedEdge(e)
        if not self.directed:
            self.addSinglesidedEdge(E(e.v, e.u))

    def addEdges(self, u, *vs):
        for v in vs:
            self.addEdge(E(u, v))

    def addSinglesidedEdge(self, e):
        if e.u not in self.edges:
            self.edges[e.u] = []

        self.edges[e.u].append(e.v)

    def DFS(self):
        self.reset()

        time = 1
        for v in self.vertices:
            if v.color == WHITE:
                time = self.DFS_VISIT(v, time)

    def DFS_VISIT(self, u, time):
        vl = []
        u.color = GRAY
        u.pi    = None
        u.d     = time
        time    += 1
        vl.append(u)

        while vl:
            adjl = []
            cu   = vl[-1]
            if cu in self.edges:
                adjl = list(self.edges[cu][cu.e:])
            deepen = False
            for adjv in adjl:
                cu.e += 1
                if adjv.color == WHITE:
                    deepen = True
                    adjv.color = GRAY
                    adjv.pi = cu
                    adjv.d  = time
                    time    += 1
                    vl.append(adjv)
                    break

            if not deepen:
                v = vl.pop()
                v.color = BLACK
                v.f = time
                time += 1

        return time

    def __str__(self):
        s=""
        s += "(%d)[" % len(self.vertices)
        if self.vertices:
            vl = list(self.vertices)
            s += "%s" % str(vl[0])
            for v in vl[1:]:
                s += ", %s" % str(v)
        s += "]"+os.linesep

        for u, vs in self.edges.items():
            s += "%s -> [" % str(u)
            vl = list(vs)
            if vl:
                s += "%s" % str(vl[0])
                for v in vl[1:]:
                    s += ", %s" % str(v)
            s += "]"+os.linesep
        return s

def test():
    g = G(True)

    m = V("m")
    n = V("n")
    o = V("o")
    p = V("p")
    q = V("q")
    r = V("r")
    s = V("s")
    t = V("t")
    u = V("u")
    v = V("v")
    w = V("w")
    x = V("x")
    y = V("y")
    z = V("z")

    g.addEdges(m, q, r, x)
    g.addEdges(n, o, q, u)
    g.addEdges(o, r, s, v)
    g.addEdges(p, o, s, z)
    g.addEdges(q, t)
    g.addEdges(r, u, y)
    g.addEdges(s, r)
    g.addEdges(u, t)
    g.addEdges(v, w, x)
    g.addEdges(y, v)
    g.DFS()

    g2 = G(True)
    q = V("q")
    r = V("r")
    s = V("s")
    t = V("t")
    u = V("u")
    v = V("v")
    w = V("w")
    x = V("x")
    y = V("y")
    z = V("z")

    g2.addEdges(q, s, t, w)
    #g2.addEdges(r, u, y)
    #g2.addEdges(s, v)
    #g2.addEdges(t, x, y)
    #g2.addEdges(u, y)
    #g2.addEdges(v, w)
    #g2.addEdges(w, s)
    #g2.addEdges(x, z)
    #g2.addEdges(y, q)
    #g2.addEdges(z, x)
    g2.DFS()

    print g2
