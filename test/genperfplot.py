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

x1 = np.linspace(0.0, 5.0)
x2 = np.linspace(0.0, 2.0)

y1 = np.cos(2 * np.pi * x1) * np.exp(-x1)
y2 = np.cos(2 * np.pi * x2)

plt.subplot(2, 1, 1)
plt.plot(x1, y1, 'o-')
plt.title('A tale of 2 subplots')
plt.ylabel('Damped oscillation')

plt.subplot(2, 1, 2)
plt.plot(x2, y2, '.-')
plt.xlabel('time (s)')
plt.ylabel('Undamped')

plt.savefig(os.path.join(os.environ["TEMP"], 'mvlab-perf.svg'), format="svg")