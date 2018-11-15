import os
import sys

thisDir = os.path.dirname(os.path.abspath(sys.argv[0]))
ln = 0
for root, dirs, files in os.walk(thisDir, topdown=True, followlinks=False):
    for file in files:
        fullPath = os.path.join(root, file)
        if fullPath.endswith(".h") or fullPath.endswith(".cpp"):
            with open(fullPath, encoding='utf-8') as f:
                for l in f:
                    ln += 1
print("There are " + str(ln) + " lines of code")
os.system("pause")