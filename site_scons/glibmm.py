import cons
def IncludeGlibMM(e):
    e.Append(CPPPATH = [r'D:\Data\glibmm-2.57.1\include\glib', r'D:\Data\glibmm-2.57.1\lib\glibmm'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\glibmm-2.57.1\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\glibmm-2.57.1\lib\release'])
    e.Append(LIBS = ['glibmm'])