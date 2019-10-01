def CHANGES(s):
    ll = []
    if 1<=s:
        cs = CHANGES(s-1)
        for c in cs:
            c = c[:]
            c.append(1)
            ll.append(c)
        if not cs:
            ll.append([1])

    if 5<=s:
        cs = CHANGES(s-5)
        for c in cs:
            c = c[:]
            c.append(5)
            ll.append(c)
        if not cs:
            ll.append([5])

    if 10<=s:
        cs = CHANGES(s-10)
        for c in cs:
            c = c[:]
            c.append(10)
            ll.append(c)
        if not cs:
            ll.append([10])

    if 20<=s:
        cs = CHANGES(s-20)
        for c in cs:
            c = c[:]
            c.append(20)
            ll.append(c)
        if not cs:
            ll.append([20])

    if 50<=s:
        cs = CHANGES(s-50)
        for c in cs:
            c = c[:]
            c.append(50)
            ll.append(c)
        if not cs:
            ll.append([50])

    return ll

def UNIQUE_CHANGES(s):
    ll = CHANGES_DP(s)
    sll = set()
    for l in ll:
        l.sort()
        sll.add(tuple(l))
    ll = list(sll)
    ll.sort(lambda l, r: len(l)-len(r))
    return ll

def CHANGES_DP(s):
    cl = [[] for i in range(0, s+1)]
    for n in range(1, s+1):
        if 1<=n:
            cs = cl[n-1]
            for c in cs:
                c = c[:]
                c.append(1)
                cl[n].append(c)
            if not cs:
                cl[n].append([1])

        if 5<=n:
            cs = cl[n-5]
            for c in cs:
                c = c[:]
                c.append(5)
                cl[n].append(c)
            if not cs:
                cl[n].append([5])

        if 10<=n:
            cs = cl[n-10]
            for c in cs:
                c = c[:]
                c.append(10)
                cl[n].append(c)
            if not cs:
                cl[n].append([10])

        if 20<=n:
            cs = cl[n-20]
            for c in cs:
                c = c[:]
                c.append(20)
                cl[n].append(c)
            if not cs:
                cl[n].append([20])

        if 50<=n:
            cs = cl[n-50]
            for c in cs:
                c = c[:]
                c.append(50)
                cl[n].append(c)
            if not cs:
                cl[n].append([50])
        sll = set()
        for l in cl[n]:
            l.sort()
            sll.add(tuple(l))
        cl[n] = [list(l) for l in sll]
        cl[n].sort(lambda l, r: len(l)-len(r))
    return cl[s]