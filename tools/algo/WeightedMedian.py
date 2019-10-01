import random
import algo
def test_WEIGHTED_MEDIAN():
    for t in range(0, 5000):
        n = random.randint(2, 20)
        A = [random.randint(1, 1000) for i in range(1, n)]
        W = [random.randint(1, 1000) for i in range(1, n)]
        s = reduce(lambda x, y: x+y, W)+0.0
        W = [w/s for w in W]
        AW = zip(A, W)
        AW.sort(lambda x, y: x[0]-y[0])

        accw = 0.0
        wm = 0.0
        for aw in AW:
            accw += aw[1]
            if accw>0.5:
                wm = aw[0]
                break

        aa = algo.WEIGHTED_MEDIAN(A, W, 0, len(A)-1)
        if wm == aa:
            print str(t) + ": Pass"
        else:
            print accw, wm, aa
            print AW
            raise AssertionError("Failed")