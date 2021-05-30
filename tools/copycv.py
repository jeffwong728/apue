import os
import sys
import shutil

def getCurDirPath():
    dirPath = os.path.dirname(sys.argv[0])
    if not dirPath:
        dirPath = os.getcwd()
    return dirPath

mvlabBinDir = os.path.join(os.path.dirname(getCurDirPath()), 'build', 'opencv', 'msvs', 'bin')
cv2BinDir = os.path.join(os.path.dirname(getCurDirPath()), 'build', 'opencv', 'msvs', 'lib', 'python3')
spamBinDir = os.path.join(os.path.dirname(getCurDirPath()), 'build', 'spam', 'msvs')
mvlabD = os.path.join(mvlabBinDir, 'Debug', 'opencv_mvlab440d.dll')
mvlabR = os.path.join(mvlabBinDir, 'Release', 'opencv_mvlab440.dll')
cv2D = os.path.join(cv2BinDir, 'Debug', 'cv2.cp38-win_amd64.pyd')
cv2R = os.path.join(cv2BinDir, 'Release', 'cv2.cp38-win_amd64.pyd')
spamD = os.path.join(spamBinDir, 'Debug')
spamR = os.path.join(spamBinDir, 'Release')
spamRD = os.path.join(spamBinDir, 'RelWithDebInfo')

shutil.copy2(mvlabD, spamD, follow_symlinks=False)
shutil.copy2(mvlabR, spamR, follow_symlinks=False)
shutil.copy2(mvlabR, spamRD, follow_symlinks=False)
shutil.copy2(cv2D, spamD, follow_symlinks=False)
shutil.copy2(cv2R, spamR, follow_symlinks=-False)
shutil.copy2(cv2R, spamRD, follow_symlinks=False)
os.system('pause')