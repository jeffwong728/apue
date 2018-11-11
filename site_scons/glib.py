import cons
def IncludeGlib(e):
    e.Append(CPPPATH = [r'D:\Data\glib-2.56.1\include', r'D:\Data\glib-2.56.1\include\glib'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\glib-2.56.1\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\glib-2.56.1\lib\release'])
    e.Append(LIBS = ['glib'])