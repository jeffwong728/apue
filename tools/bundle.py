import os
import sys

thisDir = os.path.dirname(os.path.abspath(sys.argv[0]))

newLine=""
with open("bundle.txt", 'w') as ofstream:
    for fileName in os.listdir(thisDir):
        if not fileName.endswith(".txt"):
            fullPathName = os.path.join(thisDir, fileName)
            ofstream.write(newLine+'#'*80+"\n")
            ofstream.write('#'+fullPathName+"\n")
            ofstream.write('#'*80+"\n")
            with open(fullPathName, 'r') as ifstream:
                ofstream.write(ifstream.read())
                newLine = "\n\n"