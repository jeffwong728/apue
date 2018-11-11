import cons

def IncludePython2(e):
    e.Append(CPPPATH = [r'C:\Python27\include'])
    e.Append(LIBPATH = ['C:\Python27\libs'])
    e.Append(CPPDEFINES = ['SWIG_PYTHON_INTERPRETER_NO_DEBUG'])