import os
import sys

thisDir = os.path.dirname(os.path.abspath(sys.argv[0]))
ln = 0
for root, dirs, files in os.walk(thisDir, topdown=True, followlinks=False):
    for file in files:
        if file.endswith(".CPP") and file.upper() == file:
            r = input("Rename " + file + " to " + file.lower() + "? (Y or N): ")
            r = r.strip()
            r = r.lower()
            if not r or r=="y":
                print("Rename " + file + " to " + file.lower())
                os.rename(os.path.join(root, file), os.path.join(root, file.lower()))
os.system("pause")