import os
import sys
from bs4 import BeautifulSoup

if "__main__"==__name__:
    thisDir = os.path.dirname(os.path.abspath(sys.argv[0]))
    for root, dirs, files in os.walk(thisDir, topdown=True, followlinks=False):
        for file in files:
            fullPath = os.path.join(root, file)
            if fullPath.endswith(".htm") or fullPath.endswith(".html"):
                soup = BeautifulSoup(open(fullPath), "html5lib", from_encoding="utf-8")
                with open(fullPath, 'wb') as ofstream:
                    ofstream.write(soup.prettify().encode("utf-8"))
                    print fullPath + " prettified."
    os.system("pause")