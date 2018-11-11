import cons
def IncludeCairo(e):
    e.Append(CPPPATH = [r'D:\Data\cairo-1.14.12\include'])
    e.Append(CPPDEFINES = ['CAIRO_WIN32_STATIC_BUILD'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBPATH = [r'D:\Data\cairo-1.14.12\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBPATH = [r'D:\Data\cairo-1.14.12\lib\release'])
    e.Append(LIBS = ['cairo', 'pixman'])