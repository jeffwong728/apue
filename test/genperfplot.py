import os
import sys
import numpy as np
import matplotlib.pyplot as plt

perfData = dict()
perfLogPath = os.path.join(os.environ["TEMP"], 'mvlab.log')

with open(perfLogPath, mode='r', encoding='utf8') as f:
    for line in f:
        words = line.split()
        if words[5] not in perfData:
            perfData[words[5]] = dict()
        if words[4] not in perfData[words[5]]:
            perfData[words[5]][words[4]] = list()
        perfData[words[5]][words[4]].append(words[-1])

def samplemat(dims):
    """Make a matrix with all zeros and increasing elements on the diagonal"""
    aa = np.zeros(dims)
    for i in range(min(dims)):
        aa[i, i] = i
    return aa


# Display matrix
plt.matshow(samplemat((15, 15)))
plt.savefig(os.path.join(os.environ["TEMP"], 'mvlab-perf.svg'), format="svg")