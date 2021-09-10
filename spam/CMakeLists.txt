cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(apue LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

IF (WIN32 AND MSVC)
  set(OpenCV_DIR "$ENV{SPAM_ROOT_DIR}/jane/install/opencv")
ELSE()
  set(Boost_USE_STATIC_LIBS ON)
  set(OpenCV_DIR "$ENV{SPAM_ROOT_DIR}/jane/install/opencv/lib/cmake/opencv4")
  set(wxWidgets_ROOT "$ENV{SPAM_ROOT_DIR}/jane/install/wxWidgets")
  list(APPEND CMAKE_FIND_ROOT_PATH "$ENV{SPAM_ROOT_DIR}/jane/install/wxWidgets/bin")
  list(APPEND CMAKE_PROGRAM_PATH "$ENV{SPAM_ROOT_DIR}/jane/install/wxWidgets/bin")
  set(wxWidgets_CONFIG_EXECUTABLE "$ENV{SPAM_ROOT_DIR}/jane/install/wxWidgets/bin/wx-config")
  message(STATUS "CMAKE_FIND_ROOT_PATH is ${CMAKE_FIND_ROOT_PATH}")
ENDIF()
set(2Geom_DIR "$ENV{SPAM_ROOT_DIR}/jane/install/lib2geom/lib/cmake/2Geom")
set(VTK_DIR "$ENV{SPAM_ROOT_DIR}/VTK/install/lib/cmake/vtk-9.0")

find_package(OpenGL REQUIRED)
find_package(2Geom REQUIRED)
find_package(ICU REQUIRED i18n data io uc)
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
find_package(TBB CONFIG REQUIRED tbb tbbmalloc)
find_package(double-conversion CONFIG REQUIRED)
find_package(OpenCV REQUIRED core imgproc imgcodecs mvlab)
find_package(Boost REQUIRED COMPONENTS locale system thread chrono filesystem unit_test_framework)
find_package(VTK COMPONENTS vtkCommonCore vtkCommonColor vtkCommonDataModel vtkFiltersSources vtkIOLegacy vtkIOXML
                            vtkRenderingCore vtkRenderingFreeType vtkRenderingGL2PSOpenGL2 vtkRenderingOpenGL2
                            vtkRenderingAnnotation vtkRenderingExternal vtkIOImage vtkIOGeometry vtkRenderingLabel
                            vtkFiltersCore vtkFiltersExtraction vtkFiltersGeneral vtkFiltersGeometry vtkFiltersHybrid vtkFiltersSources QUIET)

if (NOT VTK_FOUND)
  message("Skipping Config: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")

set(HDF5_USE_STATIC_LIBRARIES OFF)
IF (WIN32 AND MSVC)
  list(APPEND CMAKE_MODULE_PATH "$ENV{SPAM_ROOT_DIR}/jane/cmake")
  find_package(wxGTK3 REQUIRED COMPONENTS mono)
  find_package(VCPKG REQUIRED COMPONENTS gtk3 gdk3 gio pangocairo gdk_pixbuf pango gobject gthread glib glibmm gmodule cairo cairomm exif epoxy)
  list(POP_BACK CMAKE_MODULE_PATH)
  find_package(unofficial-glib CONFIG REQUIRED)
  find_package(unofficial-cairo CONFIG REQUIRED)
  find_package(hdf5 CONFIG REQUIRED)
  find_package(GSL REQUIRED)
  find_package(sigc++-3 CONFIG REQUIRED)
  find_package(expat CONFIG REQUIRED)
ELSE()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK3 gtk+-3.0 IMPORTED_TARGET)
  pkg_check_modules(GLIB glib-2.0 IMPORTED_TARGET)
  pkg_check_modules(GSL gsl IMPORTED_TARGET)
  pkg_check_modules(CAIRO cairo IMPORTED_TARGET)
  pkg_check_modules(CAIROMM cairomm-1.0 IMPORTED_TARGET)
  pkg_check_modules(GLIBMM glibmm-2.4 IMPORTED_TARGET)
  pkg_check_modules(SIGCPP sigc++-2.0 IMPORTED_TARGET)
  pkg_check_modules(EPOXY epoxy IMPORTED_TARGET)
  find_package(EXPAT REQUIRED)
  find_package(HDF5 REQUIRED COMPONENTS C CXX HL)
  find_package(wxWidgets REQUIRED COMPONENTS mono)
ENDIF()

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

set(PYBIND11_NOPYTHON ON)
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(glm)
add_subdirectory(pybind11)
add_subdirectory(cv_hdf)
add_subdirectory(helper)
add_subdirectory(svg)
add_subdirectory(util)
add_subdirectory(livarot)
add_subdirectory(stub)
add_subdirectory(ui)

IF (WIN32 AND MSVC)
  add_executable(spam WIN32 init.cpp msvc_recommended_pragmas.h)
  target_link_libraries(spam PRIVATE sigc-3.0 expat::expat)
ELSE()
  add_executable(spam init.cpp)
  target_link_libraries(spam PRIVATE EXPAT::EXPAT)
  target_link_libraries(spam PRIVATE PkgConfig::SIGCPP)
ENDIF()

set_target_properties (spam PROPERTIES FOLDER spam)
target_link_libraries(spam PRIVATE ui_core helper svg util livarot stub cv_hdf)

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

target_sources(spam PRIVATE ${GLIB_RESOURCES_CPP} menus.ui ${GLIB_RESOURCES_XML} res/css/spam.css)

if(WIN32)
    target_sources(spam PRIVATE spam.rc)
endif()

if(MSVC)
  target_compile_definitions(spam PRIVATE _CRTDBG_MAP_ALLOC)
  set_target_properties(spam PROPERTIES COMPILE_FLAGS "/wd4828")
endif()

include(GNUInstallDirs)
install(TARGETS spam DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:spam> DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
    include (InstallRequiredSystemLibraries)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake @ONLY)
    install (SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)
endif()

get_target_property(sys_inc ICU::i18n INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(spam SYSTEM INTERFACE ${sys_inc})
message(STATUS "VCPKG include directories: ${sys_inc}")

add_custom_command(TARGET spam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/res" "$<TARGET_FILE_DIR:spam>/res"
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/res
    COMMENT "Copying resources to bin directory")

add_custom_command(TARGET spam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/ui/graphics/glsl" "$<TARGET_FILE_DIR:spam>/res/glsl"
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ui/graphics/glsl
    COMMENT "Copying GLSL shader files to res directory")

message(STATUS "Project will be installed to ${CMAKE_INSTALL_PREFIX}")
