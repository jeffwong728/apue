import cons
def IncludeTBB(e):
    e.Append(CPPPATH = [r'D:\Data\tbb2018_20171205oss\include'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBS = ['tbb_debug', 'tbbmalloc_debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBS = ['tbb', 'tbbmalloc'])
    e.Append(LIBPATH = [r'D:\Data\tbb2018_20171205oss\lib\intel64\vc14'])
    e.Append(CPPDEFINES = [('TBB_PREVIEW_MEMORY_POOL', 1)])