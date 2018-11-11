import cons
def IncludeICU(e):
    e.Append(CPPPATH = [r'D:\Data\icu4c-61_1\include'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBS = ['icudt', 'icuind', 'icuiod', 'icutud', 'icuucd'])
        e.Append(LIBPATH = [r'D:\Data\icu4c-61_1\lib64'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBS = ['icudt', 'icuin', 'icuio', 'icutu', 'icuuc'])
        e.Append(LIBPATH = [r'D:\Data\icu4c-61_1\lib64'])