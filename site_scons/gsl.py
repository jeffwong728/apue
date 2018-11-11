import cons
def IncludeGsl(e):
    e.Append(CPPPATH = [r'D:\Data\gsl-2.4.0\include'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\gsl-2.4.0\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\gsl-2.4.0\lib\release'])
    e.Append(LIBS = ['gsl', 'gslcblas'])