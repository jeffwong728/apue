cmake_minimum_required(VERSION 3.12)
project(cairomm)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/build/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options("/utf-8")
set(CMAKE_DEBUG_POSTFIX "d")
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#get cairomm version
file (STRINGS "configure.ac" CONFIGURE_AC_INIT REGEX "AC_INIT\\(\\[cairomm\\], \\[.*\\].*" )
string(REGEX REPLACE "AC_INIT\\(\\[.*\\], \\[([0-9]+\\.[0-9]+\\.[0-9]+)\\].*" "\\1"  VERSION ${CONFIGURE_AC_INIT})
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" CAIROMM_MAJOR_VERSION ${VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" CAIROMM_MINOR_VERSION ${VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" CAIROMM_MICRO_VERSION ${VERSION})
message(STATUS "Ciaromm version: ${CAIROMM_MAJOR_VERSION}.${CAIROMM_MINOR_VERSION}.${CAIROMM_MICRO_VERSION}")

get_filename_component(zlibAbsInstallPath "../../install/zlib" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(sigcAbsInstallPath "../../install/sigc++" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(cairoAbsInstallPath "../../install/cairo" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(freetypeAbsInstallPath "../../install/freetype" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(localModuleAbsPath "../../cmake" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(fontconfigAbsInstallPath "../../install/fontconfig" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pixmanAbsInstallPath "../../install/pixman" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pngAbsInstallPath "../../install/libpng" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(bzip2AbsInstallPath "../../install/bzip2" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(iconvAbsInstallPath "../../install/libiconv" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(expatAbsInstallPath "../../install/expat" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH ${localModuleAbsPath})
set(sigc++-3_ROOT "${sigcAbsInstallPath}")
set(CAIRO_ROOT "${cairoAbsInstallPath}")
set(Freetype_ROOT "${freetypeAbsInstallPath}")
set(FontConfig_ROOT "${fontconfigAbsInstallPath}")
set(Pixman_ROOT "${pixmanAbsInstallPath}")
set(PNG_ROOT "${pngAbsInstallPath}")
set(ZLIB_ROOT "${zlibAbsInstallPath}")
set(BZip2_ROOT "${bzip2AbsInstallPath}")
set(Iconv_ROOT "${iconvAbsInstallPath}")
set(EXPAT_ROOT "${expatAbsInstallPath}")

find_package(CAIRO REQUIRED)
find_package(sigc++-3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(FontConfig REQUIRED)
find_package(Pixman REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Iconv REQUIRED)
find_package(EXPAT REQUIRED)

#configure
option(BUILD_SHARED_LIBS "Build the shared library" OFF)

option(CAIROMM_ENABLE_API_EXCEPTIONS "enable API exceptions" ON)
if(CAIROMM_ENABLE_API_EXCEPTIONS)
    set(CAIROMM_EXCEPTIONS_ENABLED ON)
else()
    set(CAIROMM_EXCEPTIONS_ENABLED OFF)
endif()

configure_file("build/cmake/cairommconfig.h.cmake" "cairommconfig.h")
configure_file("build/cmake/cairomm.rc.cmake" "cairomm.rc" @ONLY)

#build
set(cairomm_cc
    cairomm/context.cc
    cairomm/context_surface_quartz.cc
    cairomm/context_surface_win32.cc
    cairomm/context_surface_xlib.cc
    cairomm/device.cc
    cairomm/exception.cc
    cairomm/fontface.cc
    cairomm/fontoptions.cc
    cairomm/matrix.cc
    cairomm/path.cc
    cairomm/pattern.cc
    cairomm/private.cc
    cairomm/quartz_font.cc
    cairomm/quartz_surface.cc
    cairomm/region.cc
    cairomm/scaledfont.cc
    cairomm/script.cc
    cairomm/script_surface.cc
    cairomm/surface.cc
    cairomm/win32_font.cc
    cairomm/win32_surface.cc
    cairomm/xlib_surface.cc)

set(cairomm_public_h
    cairomm/cairomm.h
    cairomm/context.h
    cairomm/device.h
    cairomm/enums.h
    cairomm/exception.h
    cairomm/fontface.h
    cairomm/fontoptions.h
    cairomm/matrix.h
    cairomm/path.h
    cairomm/pattern.h
    cairomm/quartz_font.h
    cairomm/quartz_surface.h
    cairomm/refptr.h
    cairomm/region.h
    cairomm/scaledfont.h
    cairomm/script.h
    cairomm/script_surface.h
    cairomm/surface.h
    cairomm/types.h
    cairomm/win32_font.h
    cairomm/win32_surface.h
    cairomm/xlib_surface.h)

set(cairomm_private_h
    cairomm/context_private.h
    cairomm/private.h)

set(cairomm_rc
    ${CMAKE_BINARY_DIR}/cairomm.rc)

add_library(cairomm-1.0 STATIC ${cairomm_cc} ${cairomm_rc})
target_link_libraries(cairomm-1.0 PUBLIC CAIRO::cairo "sigc-3.0" Freetype::Freetype FontConfig::FontConfig Pixman::Pixman)
target_include_directories(cairomm-1.0 PUBLIC
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR})

add_executable(pdf-surface examples/surfaces/pdf-surface.cc)
target_link_libraries(pdf-surface PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

add_executable(image-surface examples/surfaces/image-surface.cc)
target_link_libraries(image-surface PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

add_executable(svg-surface examples/surfaces/svg-surface.cc)
target_link_libraries(svg-surface PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

add_executable(text-rotate examples/text/text-rotate.cc)
target_link_libraries(text-rotate PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

add_executable(toy-text examples/text/toy-text.cc)
target_link_libraries(toy-text PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

add_executable(user-font examples/text/user-font.cc)
target_link_libraries(user-font PRIVATE cairomm-1.0 PNG::PNG ZLIB::ZLIB BZip2::BZip2 EXPAT::EXPAT Iconv::Iconv Iconv::charset)

if(MSVC)
  target_compile_definitions(toy-text PUBLIC _USE_MATH_DEFINES)
  target_compile_definitions(user-font PUBLIC _USE_MATH_DEFINES)
endif()

install(
    TARGETS cairomm-1.0
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

install(
    FILES ${cairomm_public_h}
    DESTINATION include/cairomm)

install(
    FILES ${CMAKE_BINARY_DIR}/cairommconfig.h
    DESTINATION include)
