# -*- coding: utf-8 -*-
import os
import sys
import shutil

CodeDirPath = r"E:\predator_DEV_D1"
MyPublicDirPath = r"\\10.122.65.100\¹«¹²ÎÄµµ\wangweimesh"

class DirError(IOError):
    def __init__(self, dirPath):
        IOError.__init__(self)
        self.filename = dirPath

def getCurDirPath():
    dirPath = os.path.dirname(sys.argv[0])
    if not dirPath:
        dirPath = os.getcwd()
    return dirPath
    
def get7zRootPath():
    probePathes = []
    probePathes.append(r"C:\Program Files\7-Zip")
    probePathes.append(r"D:\Program Files\7-Zip")  
    for probPath in probePathes:
        if os.path.exists(os.path.join(probPath, "7z.exe")):
            return probPath
    
    print "Can't locate 7z program file."
    return None
    
def compressFEMPlugin():
    FEMPluginDirPath = os.path.join(getCurDirPath(), "FEMPlugin")
    print "Start compress " + FEMPluginDirPath + "..."
    
    path7Z = get7zRootPath()
    if not path7Z:
        return
        
    if path7Z not in os.environ["PATH"]:
        os.environ["PATH"] = path7Z+";"+ os.environ["PATH"]
        
    compressCmd = r"7z.exe a FEMPlugin.7z FEMPlugin\ -xr0!*.pdb"
    os.system(compressCmd)
    print "Compress " + FEMPluginDirPath + " completed."
    
def copyFEMPlugin():
    srcFEMPath = os.path.join(getCurDirPath(), "FEMPlugin.7z")
    dstFEMPath = os.path.join(MyPublicDirPath, "FEMPlugin.7z")
    
    if os.path.exists(srcFEMPath):
        if os.path.exists(dstFEMPath):
            os.remove(dstFEMPath)
        print "Copying FEMPlugin.7z ..."
        shutil.copy(srcFEMPath, dstFEMPath)
        print "Copy FEMPlugin.7z completed."
        
def deleteFEMPluginArchive():
    femArchivePath = os.path.join(getCurDirPath(), "FEMPlugin.7z")
    os.remove(femArchivePath)
    
if "__main__"==__name__:
    compressFEMPlugin()
    copyFEMPlugin()
    deleteFEMPluginArchive()
    os.system("pause")