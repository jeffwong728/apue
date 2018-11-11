import cons
def IncludeHDF5(e):
    e.Append(CPPPATH = [r'D:\Data\hdf5-1.10.2\include'])
    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(LIBS = ['libhdf5_D', 'libhdf5_cpp_D', 'libhdf5_hl_D', 'libhdf5_hl_cpp_D', 'libszip_D', 'libzlib_D'])
        e.Append(LIBPATH = [r'D:\Data\hdf5-1.10.2\lib\debug'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        e.Append(LIBS = ['libhdf5', 'libhdf5_cpp', 'libhdf5_hl', 'libhdf5_hl_cpp', 'libszip', 'libzlib'])
        e.Append(LIBPATH = [r'D:\Data\hdf5-1.10.2\lib\release'])