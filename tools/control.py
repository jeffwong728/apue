import os
import sys

thisDir = r'D:\Vcpkg\ports'
newLine=""
with open("bundle.txt", 'w') as ofstream:
    for fileName in os.listdir(thisDir):
        fullPathName = os.path.join(thisDir, fileName, 'CONTROL')
        ofstream.write(newLine+fileName+'#'*80+"\n")
        with open(fullPathName, 'r') as ifstream:
            ofstream.write(ifstream.read())
            newLine = "\n\n"