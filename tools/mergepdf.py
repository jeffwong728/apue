import os
import sys
import PyPDF2

def getCurDirPath():
    dirPath = os.path.dirname(sys.argv[0])
    if not dirPath:
        dirPath = os.getcwd()
    return dirPath
    

allFiles = os.listdir(getCurDirPath())
allPDFs = [f for f in allFiles if f.endswith('.pdf')]
allPDFs.sort()

oPDF = PyPDF2.PdfFileWriter()
for f in allPDFs:
    fPDF = open(f, "rb")
    iPDF = PyPDF2.PdfFileReader(fPDF)
    for p in range(iPDF.numPages):
        page = iPDF.getPage(p)
        oPDF.addPage(page)

oPDF.write(open("all.pdf", "wb"))
os.system("pause")