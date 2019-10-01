import os
import sys
import math
import bisect
import random
from cStringIO import StringIO

def listPath():
    pathstr = os.environ["PATH"]
    pathList = pathstr.split(";")
    for p in pathList:
        print p

def poly(a, x):
    y = a[-1]
    r = range(-2, -len(a)-1, -1)
    for i in r:
        y = a[i]+x*y
    return y

def COUNT_INVERSIONS(A, p, r):
    inversions = 0
    if p<r:
        q=(p+r)/2
        inversions += COUNT_INVERSIONS(A, p, q)
        inversions += COUNT_INVERSIONS(A, q+1, r)
        inversions += MERGE_INVERSIONS(A, p, q, r)
    
    return inversions

def MERGE_INVERSIONS(A, p, q, r):
    nL = q-p+1
    L  = A[p:q+1]
    R  = A[q+1:r+1]

    L.append(sys.maxint)
    R.append(sys.maxint)

    i = 0
    j = 0
    inversions = 0

    for k in range(p, r+1):
        if R[j] < L[i]:
            A[k] = R[j]
            inversions += nL-i
            j += 1
        else:
            A[k] = L[i]
            i += 1

    return inversions

def FETCH_BIT(n, c):
    return n & (1<<c);

def FIND_MISSING(a, *c):
    if c:
        c = c[0]
    else:
        c = 0

    if not len(a):
        return 0

    zeros = []
    ones  = []

    for n in a:
        if FETCH_BIT(n, c):
            ones.append(n)
        else:
            zeros.append(n)

    if len(zeros)<=len(ones):
        v = FIND_MISSING(zeros, c+1)
        return v<<1 | 0
    else:
        v = FIND_MISSING(ones, c+1)
        return v<<1 | 1

def LEFT(i):
    return i*2+1

def RIGHT(i):
    return i*2+2

def PARENT(i):
    return (i-1)/2

def MAX_HEAPIFY(A, i):
    n=len(A)
    l = LEFT(i)
    r = RIGHT(i)
    largest = i
    if l<n and A[l]>A[largest]:
        largest = l

    if r<n and A[r]>A[largest]:
        largest = r

    if not largest == i:
        A[i], A[largest] = A[largest], A[i]
        MAX_HEAPIFY(A, largest)

def MAX_HEAPIFY_LOOP(A, i):
    n=len(A)
    while i<n/2:
        l = LEFT(i)
        r = RIGHT(i)
        largest = i
        if l<n and A[l]>A[largest]:
            largest = l

        if r<n and A[r]>A[largest]:
            largest = r

        if largest == i:
            break

        A[i], A[largest] = A[largest], A[i]
        i = largest

def BUILD_MAX_HEAP(A):
    r = range(len(A)/2-1, -1, -1)
    for i in r:
        MAX_HEAPIFY(A, i)

def BUILD_MAX_HEAP_LOOP(A):
    r = range(len(A)/2-1, -1, -1)
    for i in r:
        MAX_HEAPIFY_LOOP(A, i)

def QUICKSORT(A, p, r):
    if p<r:
        q = PARTITION(A, p, r)
        QUICKSORT(A, p, q-1)
        QUICKSORT(A, q+1, r)

def PARTITION(A, p, r, W=None):
    x = A[r]
    i = p-1
    for j in range(p, r):
        if A[j]<=x:
            i += 1
            A[i], A[j] = A[j], A[i]
            W[i], W[j] = W[j], W[i]
    A[i+1], A[r] = A[r], A[i+1]
    W[i+1], W[r] = W[r], W[i+1]
    return i+1

def RANDOMIZED_PARTITION(A, p, r, W=None):
    i = random.randint(p, r)
    A[i], A[r] = A[r], A[i]
    W[i], W[r] = W[r], W[i]
    return PARTITION(A, p, r, W)

def QUICKSORT_EQUAL_RANG(A, p, r):
    if p<r:
        q, e = PARTITION_EQUAL_RANGE(A, p, r)
        QUICKSORT_EQUAL_RANG(A, p, q-1)
        QUICKSORT_EQUAL_RANG(A, e+1, r)

def PARTITION_EQUAL_RANGE(A, p, r):
    x = A[r]
    i = p-1
    k = p-1
    for j in range(p, r):
        if A[j]<x:
            i += 1
            k += 1
            A[k], A[j] = A[j], A[k]
            A[k], A[i] = A[i], A[k]
        if A[j]==x:
            k += 1
            A[k], A[j] = A[j], A[k]
    A[k+1], A[r] = A[r], A[k+1]
    return i+1, k+1

def TAIL_RECURSIVE_QUICKSORT(A, p, r):
    while p<r:
        q = PARTITION(A, p, r)
        if q-p < r-q:
            TAIL_RECURSIVE_QUICKSORT(A, p, q-1)
            p = q+1
        else:
            TAIL_RECURSIVE_QUICKSORT(A, q+1, r)
            r = q-1

def MATCH_JUGS(R, B):
    if not R:
        return

    if not len(R)==len(B):
        return

    if len(R)==1:
        print R[0], B[0]

    r   = R[0]
    bl  = []
    bg  = []
    for b in B:
        if b < r:
            bl.append(b)
        elif b> r:
            bg.append(b)
        else:
            print r, b

    rl = []
    rg = []
    for q in R[1:]:
        if q < r:
            rl.append(q)
        elif q > r:
            rg.append(q)

    MATCH_JUGS(rl, bl)
    MATCH_JUGS(rg, bg)

def FIND_MIN_MAX(A):
    if not A:
        return

    n = len(A)
    if n&1:
        p   = 1
        min = A[0]
        max = A[0]
    else:
        p   = 2
        if A[0]<A[1]:
            min = A[0]
            max = A[1]
        else:
            min = A[1]
            max = A[0]

    for i in range(p, n, 2):
        if A[i] < A[i+1]:
            if A[i] < min:
                min = A[i]
            if A[i+1] > max:
                max = A[i+1]
        else:
            if A[i+1] < min:
                min = A[i+1]
            if A[i] > max:
                max = A[i]

    return min, max

def MIN(a, b):
    return a if a<b else b

def SECOND_SMALLEST(A):
    if len(A)<2:
        return
    B = [A[:]]
    for r in range(0, len(A)):
        B.append([sys.maxint]*len(A))

    n = len(A)
    r = 0

    while n>1:
        for i in range(0, n, 2):
            B[r+1][i] = MIN(B[r][i], B[r][i+1])
        n = (n+1)/2
        r = r+1

def RANDOMIZED_SELECT(A, p, r, i, W):
    if p==r:
        return A[p]

    q = RANDOMIZED_PARTITION(A, p, r, W)
    k = q-p+1

    if i==k:
        return A[q]
    elif i<k:
        return RANDOMIZED_SELECT(A, p, q-1, i, W)
    else:
        return RANDOMIZED_SELECT(A, q+1, r, i-k, W)

def RANDOMIZED_SELECT_LOOP(A, p, r, i):
    while not r<p:
        q = RANDOMIZED_PARTITION(A, p, r)
        k = q-p+1

        if i==k:
            return A[q]
        elif i<k:
            r=  q-1
        else:
            p, i =  q+1, i-k

def KTH_QUANTILES(A, p, r, k):
    if r<p or k<=1:
        return

    n = r-p+1
    if not n%k:
        m = n/k
    else:
        m = int(math.ceil((n+k-1.0)/k)-1)

    q = RANDOMIZED_SELECT(A, p, r, m*((k+1)/2))
    print A[p+m*((k+1)/2)-1]

    KTH_QUANTILES(A, p, m*((k+1)/2)-1, (k+1)/2)
    KTH_QUANTILES(A, m*((k+1)/2), r, (k-1)/2)

def K_NEARNEAST_NEIGHBOUR(A, r, k):
    q = RANDOMIZED_SELECT(A, 0, len(A)-1, len(A)/2)
    B = [abs(v-q) for v in A]
    B[len(A)/2-1], B[-1] = B[-1], B[len(A)/2-1]
    print B

def IS_MEDIAN(A, B, k):
    n = len(A)
    if k == n-1:
        return A[k]<=B[0]
    if B[n-k-2]<=A[k] and A[k]<=B[n-k-1]:
        return True
    else:
        return False

def TWO_ARRAY_MEDIAN_LOOP(A, B):
    i = 0
    j = len(A)-1
    m = 0
    n = len(B)-1

    while (j-i+1+n-m+1)>4:
        k = (i+j)/2
        q = (n+m)/2

        if A[k]<B[q]:
            n = n-k+i
            i = k
        else:
            j = j-q+m
            m = q

    if IS_MEDIAN(A, B, i):
        return A[i]

    if IS_MEDIAN(A, B, j):
        return A[j]

    if IS_MEDIAN(B, A, m):
        return B[m]

    if IS_MEDIAN(B, A, n):
        return B[n]

def TWO_ARRAY_MEDIAN(A, B):
    median = FIND_MEDIAN(A, B, 0, len(A)-1)
    if not median == sys.maxint:
        return median
    else:
        return FIND_MEDIAN(B, A, 0, len(B)-1)

def FIND_MEDIAN(A, B, low, high):
    if low > high:
        return sys.maxint

    n = len(A)
    k = (low+high)/2
    if IS_MEDIAN(A, B, k):
        return A[k]
    elif A[k] < B[n-k-2]:
        return FIND_MEDIAN(A, B, k+1, high)
    else:
        return FIND_MEDIAN(A, B, low, k-1)

def WEIGHTED_MEDIAN(A, W, p, r):
    n = r-p+1
    if n==1:
        return A[p]

    if n==2:
        if not W[p]<W[r]:
            return A[p]
        else:
            return A[r]

    k = (n+1)/2
    median = RANDOMIZED_SELECT(A, p, r, k, W)

    WL = 0.0
    for w in W[p:(p+k-1)]:
        WL += w

    WR = 0.0
    for w in W[(p+k):(r+1)]:
        WR += w

    if WL<0.5 and WR<0.5:
        return median
    elif WL>0.5:
        W[p+k-1] += WR
        return WEIGHTED_MEDIAN(A, W, p, p+k-1)
    else:
        W[p+k-1] += WL
        return WEIGHTED_MEDIAN(A, W, p+k-1, r)

def SUB_SET(A):
    if not len(A):
        return [[]]

    ss = SUB_SET(A[1:])
    for s in ss[:]:
        ss.append([A[0]]+s)

    return ss

def PERMUTATIONS(A):
    if 1==len(A):
        return [[A[0]]]
    r = []
    for i in range(0, len(A)):
        remainder = A[0:i]+A[(i+1):]
        for p in PERMUTATIONS(remainder):
            p.append(A[i])
            r.append(p)
    return r

def PARENTHESES_IMPL(A, l, r, c, R):
    if l<0 or r<l:
        return

    if not l and not r:
        R.append("".join(A))

    if l>0:
        A[c] = "("
        PARENTHESES_IMPL(A, l-1, r, c+1, R)

    if r>l:
        A[c] = ")"
        PARENTHESES_IMPL(A, l, r-1, c+1, R)

def PARENTHESES(n):
    A = [""]*(2*n)
    R = []
    PARENTHESES_IMPL(A, n, n, 0, R)
    return R

def SPIRAL(n):
    A = []
    for i in range(0, n):
        A.append([0]*n)

    c = 1
    for l in range(0, n/2):
        for j in range(l, n-l-1):
            A[l][j] = c
            c += 1
        for j in range(l, n-l-1):
            A[j][n-1-l] = c
            c += 1
        for j in range(n-l-1, l, -1):
            A[n-1-l][j] = c
            c += 1
        for j in range(n-l-1, l, -1):
            A[j][l] = c
            c += 1
    if n & 1:
        A[n/2][n/2] = c

    s=""
    for i in range(0, n):
        for j in range(0, n):
            s += '%3d ' % A[i][j]
        s += os.linesep
    return s

def PHONE_NUMBER_IMPL(A, N, B, n):
    if n==len(N):
        print B
    else:
        for c in A[N[n]]:
            B[n] = c
            PHONE_NUMBER_IMPL(A, N, B, n+1)

def PHONE_NUMBER(N):
    A = ['0', '1', 'ABC', 'DEF', 'GHI', 'JKL', 'MNO', 'PQRS', 'TUV', 'WXYZ']
    B = ['']*len(N)
    PHONE_NUMBER_IMPL(A, N, B, 0)

def LIS_PRINT(A, P, e):
    if e<0:
        return
    else:
        LIS_PRINT(A, P, P[e])
        print A[e]

def LIS(A):
    maxLength = 1
    bestEnd = 0

    DP = [0]*len(A)
    prev = [0]*len(A)

    DP[0] = 1
    prev[0] = -1

    for i in range(1, len(A)):
        DP[i] = 1
        prev[i] = -1

        for j in range(i - 1, -1, -1):
            if DP[j] + 1 > DP[i] and A[j] < A[i]:
                DP[i] = DP[j] + 1
                prev[i] = j

        if (DP[i] > maxLength):
            bestEnd = i
            maxLength = DP[i]

    LIS_PRINT(A, prev, bestEnd)

def LIS_LOG(A):
    E = [A[0]]
    L = [A[0]]
    for a in A:
        if a>E[-1]:
            E.append(a)
            L.append(a)
        else:
            i = bisect.bisect_left(E, a)
            E[i] = a
            if i==len(E)-1:
                L[-1] = a

    return L