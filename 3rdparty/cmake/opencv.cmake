set(CMAKE_SKIP_INSTALL_RPATH OFF CACHE BOOL "" FORCE)
set(CMAKE_SKIP_RPATH OFF CACHE BOOL "" FORCE)
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" FORCE)

set(MIN_VER_CMAKE 3.15 CACHE STRING "CMake version" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Build unit tests" FORCE)
set(BUILD_PERF_TESTS OFF CACHE BOOL "Build performance tests" FORCE)
set(BUILD_WITH_STATIC_CRT OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_DEBUG_INFO ON CACHE BOOL "" FORCE)
set(USE_WIN32_FILEIO ON CACHE BOOL "" FORCE)
set(WITH_OPENMP ON CACHE BOOL "" FORCE)
set(WITH_TBB ON CACHE BOOL "" FORCE)
set(WITH_Eigen3 ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_legacy OFF CACHE BOOL "" FORCE)
set(BUILD_NEW_PYTHON_SUPPORT ON CACHE BOOL "" FORCE)
set(BUILD_opencv_python2 OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python3 ON CACHE BOOL "" FORCE)

set(OPENCV_EXTRA_MODULES_PATH "D:/apue/apex/modules" CACHE PATH "OpenCV contrib directory" FORCE)
set(OpenBLAS_LIB "D:/apue/install/OpenBLAS/lib/libopenblas.lib" CACHE PATH "OpenBLAS lib file" FORCE)

set(Python3_ROOT_DIR "C:/Python37" CACHE PATH "Python3 root directory" FORCE)
set(Python_ADDITIONAL_VERSIONS 3.7 CACHE STRING "Python3 version to search" FORCE)
set(PYTHON_DEFAULT_EXECUTABLE "C:/Python37/python.exe" CACHE FILEPATH "Python default interpreter" FORCE)
set(PYTHON3_INCLUDE_DIR "C:/Python37/include" CACHE PATH "Python3 include directory" FORCE)
#set(PYTHON3_LIBRARIES "optimized;‪C:/Python37/libs/python37.lib;debug;C:/Python37/libs/python37.lib"  CACHE FILEPATH "Python3 library" FORCE)
set(PYTHON3_LIBRARY_RELEASE "C:/Python37/libs/python37.lib" CACHE FILEPATH "Python3 library" FORCE)
set(PYTHON3_LIBRARY_DEBUG "C:/Python37/libs/python37.lib" CACHE FILEPATH "Python3 debug library" FORCE)
set(PYTHON3_EXECUTABLE "C:/Python37/python.exe" CACHE FILEPATH "Python3 interpreter" FORCE)
set(PYTHON3_PACKAGES_PATH "C:/Python37/Lib/site-packages" CACHE PATH "Python3 site-packages directory" FORCE)

set(CMAKE_INSTALL_PREFIX "D:/apue/install/opencv" CACHE PATH "install directory prefix" FORCE)

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif