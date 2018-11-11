def IncludeBoost(e):
    e.Append(CPPDEFINES = ['BOOST_LOCALE_WITH_ICU'])
    e.Append(CPPPATH = [r'D:\Data\boost_1_67_0\include\boost-1_67'])
    e.Append(LIBPATH = [r'D:\Data\boost_1_67_0\lib'])