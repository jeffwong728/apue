import os
import ycm_core

libPathFlags =  r'/LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\amd64 '
libPathFlags += r'/LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\atlmfc\lib\amd64 '
libPathFlags += r'/LIBPATH:"C:\Program Files (x86)\Windows Kits\10\lib\10.0.10240.0\ucrt\x64 '
libPathFlags += r'/LIBPATH:"C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\um\x64 '
libPathFlags += r'/LIBPATH:"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.1\Lib\um\x64 '
libPathFlags += r'/LIBPATH:"D:\ComponentCache\scasystem\REL7\525356\win64\WIN8664\lib"'

flags = [
'-std=c++11',
'-x',
'c++',
'--target=x86_64-pc-windows-msvc19.12.25831',
'-I',
r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include',
'-I',
r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\atlmfc\include',
'-I',
r'C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\ucrt',
'-I',
r'C:\Program Files (x86)\Windows Kits\8.1\Include\um',
'-I',
r'C:\Program Files (x86)\Windows Kits\8.1\Include\shared',
'-I',
r'C:\Program Files (x86)\Windows Kits\8.1\Include\winrt',
'-I',
r'D:\ComponentCache\scasystem\REL7\525356\win64\WIN8664\include',
'/link',
libPathFlags,
'/EHsc',
]

compilation_database_folder = ''

if os.path.exists( compilation_database_folder ):
  database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
  database = None

SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', '.m', '.mm' ]

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )


def IsHeaderFile( filename ):
  extension = os.path.splitext( filename )[ 1 ]
  return extension in [ '.h', '.hxx', '.hpp', '.hh' ]


def GetCompilationInfoForFile( filename ):
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if os.path.exists( replacement_file ):
        compilation_info = database.GetCompilationInfoForFile(
          replacement_file )
        if compilation_info.compiler_flags_:
          return compilation_info
    return None
  return database.GetCompilationInfoForFile( filename )


def FlagsForFile( filename, **kwargs ):
  if not database:
    return {
      'flags': flags,
      'include_paths_relative_to_dir': DirectoryOfThisScript()
    }

  compilation_info = GetCompilationInfoForFile( filename )
  if not compilation_info:
    return None

  final_flags = list( compilation_info.compiler_flags_ )

  return {
    'flags': final_flags,
    'include_paths_relative_to_dir': compilation_info.compiler_working_dir_
  }
