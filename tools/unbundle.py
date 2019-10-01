import os
import sys

thisDir = os.path.dirname(os.path.abspath(sys.argv[0]))
statements = []
pathNames = []
with open("bundle.txt", 'r') as ifstream:
    lines = list(ifstream)
    if len(lines)>3:
        i = 0;
        while i < len(lines)-2:
            if lines[i]=="#"*80+"\n" and lines[i+1].startswith("#") and lines[i+2]=="#"*80+"\n":
                pathName = os.path.basename(lines[i+1][1:])
                pathNames.append(pathName[0:-1])
                statements.append(i-1)
                statements.append(i+3)
                i += 3
            else:
                i += 1

    statements.append(len(lines)-1)
    for (s, e, pathName) in zip(statements[1::2], statements[2::2], pathNames):
        if pathName != "unbundle.py":
            with open(pathName, 'w') as ofstream:
                if lines[e]=="\n":
                    ofstream.writelines(lines[s:e-1])
                    ofstream.write(lines[e-1].rstrip("\n"))
                else:
                    ofstream.writelines(lines[s:e])
                    ofstream.write(lines[e].rstrip("\n"))