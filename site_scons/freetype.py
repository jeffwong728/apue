import cons
def IncludeFreeType(e):
    e.Append(CPPPATH = [r'D:\Data\freetype-2.9.1\include'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\freetype-2.9.1\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\freetype-2.9.1\lib\release'])
    e.Append(LIBS = ['freetype'])