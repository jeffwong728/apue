cmake_minimum_required (VERSION 3.10)
project (Tutorial)
include(CTest)

add_definitions(-DUNICODE -D_UNICODE -U_MBCS)
 
# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)
 
# does this system provide the log and exp functions?
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
 
check_function_exists (log HAVE_LOG)
check_function_exists (exp HAVE_EXP)
 
# should we use our own math functions
option(USE_MYMATH "Use tutorial provided math implementation" ON)

set(BOOST_ROOT "D:/Data/boost_1_67_0")
set(BOOST_INCLUDEDIR "D:/Data/boost_1_67_0/include/boost-1_67")
set(BOOST_LIBRARYDIR "D:/Data/boost_1_67_0/lib")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.67 REQUIRED COMPONENTS program_options date_time filesystem system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
add_definitions(${wxWidgets_DEFINITIONS})

set(wxWidgets_ROOT_DIR "D:/Data/wxWidgets-3.1.1")
set(wxWidgets_LIB_DIR "D:/Data/wxWidgets-3.1.1/lib/vc_x64_lib")
set(wxWidgets_CONFIGURATION mswu)
set(wxWidgets_EXCLUDE_COMMON_LIBRARIES OFF)
find_package(wxWidgets REQUIRED net gl core base adv aui)
include_directories(SYSTEM ${wxWidgets_INCLUDE_DIRS})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
  "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories ("${PROJECT_BINARY_DIR}")
 
# add the MathFunctions library?
if (USE_MYMATH)
  include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
  add_subdirectory (MathFunctions)
  set (EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
endif (USE_MYMATH)
 
# add the executable
add_executable (Tutorial tutorial.cxx)
target_link_libraries (Tutorial  ${EXTRA_LIBS} ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES})
 
# add the install targets
install (TARGETS Tutorial DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"        
         DESTINATION include)
 
# does the application run
add_test (TutorialRuns Tutorial 25)
 
# does the usage message work?
add_test (TutorialUsage Tutorial)
set_tests_properties (TutorialUsage
  PROPERTIES 
  PASS_REGULAR_EXPRESSION "Usage:.*number"
  )
 
 
#define a macro to simplify adding tests
macro (do_test arg result)
  add_test (TutorialComp${arg} Tutorial ${arg})
  set_tests_properties (TutorialComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endmacro (do_test)
 
# do a bunch of result based tests
do_test (4 "4 is 2")
do_test (9 "9 is 3")
do_test (5 "5 is 2.236")
do_test (7 "7 is 2.645")
do_test (25 "25 is 5")
do_test (-25 "-25 is 0")
do_test (0.0001 "0.0001 is 0.01")