import os
import cons

def IncludeOpenCV(e):
    cvModulesPath = r"D:\Data\opencv-3.4.1\modules"
    cvModules = ["videostab", "stitching", "calib3d", "features2d", "superres", "highgui"]
    cvModules += ["videoio", "shape", "imgcodecs", "dnn", "video", "photo"]
    cvModules += ["objdetect", "ml", "imgproc", "flann", "core", "world"]

    cvIncPaths = []
    for cvModule in cvModules:
        cvIncPaths.append(os.path.join(cvModulesPath, cvModule, 'include'))
    cvIncPaths.append(r'D:\Data\opencv')
    cvIncPaths.append(r'D:\Data\opencv\3rdparty\ippicv\ippicv_win\include')
    cvIncPaths.append(r'D:\Data\opencv\3rdparty\ippicv\ippiw_win\include')
    e.Append(CPPPATH = cvIncPaths)
    e.Append(LIBPATH = [r'D:\Data\opencv\3rdparty\ippicv\ippicv_win\lib\intel64'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\opencv\3rdparty\lib\Debug'])
        e.Append(LIBPATH = [r'D:\Data\opencv\lib\Debug'])
        e.Append(LIBS = ['ippiwd.lib', 'ippicvmt.lib', 'opencv_world341d.lib'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\opencv\3rdparty\lib\Release'])
        e.Append(LIBPATH = [r'D:\Data\opencv\lib\Release'])
        e.Append(LIBS = ['ippiw.lib', 'ippicvmt.lib', 'opencv_world341.lib'])