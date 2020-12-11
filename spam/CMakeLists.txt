cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(apue LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(OpenCV_DIR "$ENV{SPAM_ROOT_DIR}/install/opencv")
set(2Geom_DIR "$ENV{SPAM_ROOT_DIR}/install/lib2geom/lib/cmake/2Geom")

list(APPEND CMAKE_MODULE_PATH "$ENV{SPAM_ROOT_DIR}/cmake")
find_package(wxGTK3 REQUIRED COMPONENTS mono)
find_package(VCPKG REQUIRED COMPONENTS gtk3 gdk3 gio pangocairo gdk_pixbuf pango gobject gthread glib glibmm gmodule cairo cairomm exif)
list(POP_BACK CMAKE_MODULE_PATH)

find_package(ICU REQUIRED i18n data io uc)
find_package(Boost REQUIRED locale system thread python38 chrono filesystem unit_test_framework)
find_package(Python3 REQUIRED COMPONENTS Development)
find_package(OpenCV REQUIRED core imgproc imgcodecs mvlab)
find_package(2Geom REQUIRED)
find_package(TBB CONFIG REQUIRED tbb tbbmalloc)
find_package(hdf5 CONFIG REQUIRED)
find_package(unofficial-glib CONFIG REQUIRED)
find_package(unofficial-cairo CONFIG REQUIRED)
find_package(double-conversion CONFIG REQUIRED)
find_package(GSL REQUIRED)
find_package(sigc++-3 CONFIG REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
#target_link_libraries(main PRIVATE unofficial::glib::gio unofficial::glib::glib unofficial::glib::gmodule unofficial::glib::gobject)
#target_link_libraries(main PRIVATE unofficial::cairo::cairo unofficial::cairo::cairo-gobject)
#target_link_libraries(main PRIVATE double-conversion::double-conversion)
#target_link_libraries(main PRIVATE GSL::gsl GSL::gslcblas)
#target_link_libraries(main PRIVATE mylib pybind11::embed pybind11::module pybind11::pybind11)

message(STATUS "Found the following Python 3 libraries:")
message(STATUS "    Python 3 Version: ${Python3_VERSION}")
message(STATUS "    Python 3 include directories: ${Python3_INCLUDE_DIRS}")
message(STATUS "    Python 3 libraries: ${Python3_LIBRARIES}")
message(STATUS "    Python 3 debug library: ${Python3_LIBRARY_DEBUG}")
message(STATUS "    Python 3 release library: ${Python3_LIBRARY_RELEASE}")
message(STATUS "    Python 3 link options: ${Python3_LINK_OPTIONS}")
message(STATUS "    Python 3 library directories: ${Python3_LIBRARY_DIRS}")
message(STATUS "    Python 3 runtime library directories: ${Python3_RUNTIME_LIBRARY_DIRS}")

if(MSVC)
    add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
    add_definitions(-DUNICODE -D_UNICODE -U_MBCS)
    add_compile_options("/utf-8")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(cv_hdf)
add_subdirectory(helper)
add_subdirectory(svg)
add_subdirectory(util)
add_subdirectory(livarot)
add_subdirectory(stub)
add_subdirectory(ui)

add_executable(spam WIN32 init.cpp msvc_recommended_pragmas.h)
set_target_properties (spam PROPERTIES FOLDER spam)
target_link_libraries(spam PRIVATE ui_core helper svg util livarot stub cv_hdf sigc-3.0 expat::expat)

# (re-)generate resources file with glib-compile-resources if it's available
SET(GLIB_RESOURCES_XML "${CMAKE_CURRENT_SOURCE_DIR}/spam.gresource.xml")
SET(GLIB_RESOURCES_CPP "${CMAKE_CURRENT_SOURCE_DIR}/spam_resources.c")
find_program(GLIB_RESOURCES_COMPILER NAMES glib-compile-resources HINTS "$ENV{VCPKG_ROOT_DIR}/installed/x64-windows/tools/glib")
if(GLIB_RESOURCES_COMPILER)
    message(STATUS "Found glib-compile-resources in ${GLIB_RESOURCES_COMPILER}. ${GLIB_RESOURCES_CPP} will be recreated from ${GLIB_RESOURCES_XML}.")
    add_custom_command(
      OUTPUT
        ${GLIB_RESOURCES_CPP}
      COMMAND
        ${GLIB_RESOURCES_COMPILER} --target=${GLIB_RESOURCES_CPP} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-source ${GLIB_RESOURCES_XML}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS
        menus.ui ${GLIB_RESOURCES_XML}
      )
else()
    message(FATAL_ERROR "glib-compile-resources NOT found. Stop generate.")
endif()

target_sources(spam PRIVATE ${GLIB_RESOURCES_CPP} menus.ui ${GLIB_RESOURCES_XML})

if(WIN32)
    target_sources(spam PRIVATE spam.rc)
endif()

if(MSVC)
  target_compile_definitions(spam PRIVATE _CRTDBG_MAP_ALLOC)
endif()

include(GNUInstallDirs)
install(TARGETS spam DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:spam> DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
endif()

include (InstallRequiredSystemLibraries)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake @ONLY)
install (SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Project will be installed to ${CMAKE_INSTALL_PREFIX}")