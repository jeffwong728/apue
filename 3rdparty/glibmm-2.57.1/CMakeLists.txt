cmake_minimum_required(VERSION 3.12)
project(glibmm)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF)
set(WARNINGS_HEADER ${CMAKE_CURRENT_LIST_DIR}/msvc_recommended_pragmas.h)
add_compile_options("/utf-8")
set(CMAKE_DEBUG_POSTFIX "d")

get_filename_component(glibAbsInstallPath "../../install/glib" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pcreAbsInstallPath "../../install/PCRE" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(iconvAbsInstallPath "../../install/libiconv" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(sigcAbsInstallPath "../../install/sigc++" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(ffiAbsInstallPath "../../install/libffi" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(intlAbsInstallPath "../../install/gettext" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(localModuleAbsPath "../../cmake" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH ${localModuleAbsPath})

set(GLIB_ROOT "${glibAbsInstallPath}")
set(PCRE_ROOT "${pcreAbsInstallPath}")
set(Iconv_ROOT "${iconvAbsInstallPath}")
set(sigc++-3_ROOT "${sigcAbsInstallPath}")
set(FFI_ROOT "${ffiAbsInstallPath}")
set(Intl_ROOT "${intlAbsInstallPath}")

find_package(GLIB REQUIRED)
find_package(PCRE REQUIRED)
find_package(Iconv REQUIRED)
find_package(sigc++-3 CONFIG REQUIRED)
find_package(FFI REQUIRED)
find_package(Intl REQUIRED)

set(LIBINTL_INCLUDE_DIR ${Intl_INCLUDE_DIR})
set(LIBINTL_LIBRARY ${Intl_LIBRARY})

include_directories(${GLIB_INCLUDE_DIR})
link_libraries(
    GLIB::glib
    GLIB::gobject
    GLIB::gio
    GLIB::gthread
    GLIB::gmodule
    PCRE::pcre
    "sigc-3.0"
    FFI::FFI
)

if(APPLE)
  find_library(LIBINTL_LIBRARY NAMES intl)
endif()

#find_package(unofficial-iconv REQUIRED)
link_libraries(Intl::Intl Iconv::Iconv Iconv::charset)

if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(CORESERVICES_LIBRARY CoreServices)
  find_library(FOUNDATION_LIBRARY Foundation)
  link_libraries(${COREFOUNDATION_LIBRARY} ${CORESERVICES_LIBRARY} ${FOUNDATION_LIBRARY})
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

if(BUILD_SHARED_LIBS)
    add_definitions(-DGLIBMM_DLL -DGIOMM_DLL)
else()
    add_definitions(-DGLIBMM_STATIC_LIB -DGIOMM_STATIC_LIB)
endif()

if(WIN32)
    add_compile_options(/FI${WARNINGS_HEADER} -DSIZEOF_WCHAR_T=2)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    link_libraries(Threads::Threads ${CMAKE_DL_LIBS})
endif()

if(WIN32)
    configure_file(MSVC_Net2017/giomm/giommconfig.h ${CMAKE_BINARY_DIR}/config/gio/giommconfig.h COPYONLY)
    configure_file(MSVC_Net2017/glibmm/glibmmconfig.h ${CMAKE_BINARY_DIR}/config/glib/glibmmconfig.h COPYONLY)
else()
    set(ENV{GLIB_COMPILE_SCHEMAS} "${GLIB_COMPILE_SCHEMAS}")
    set(ENV{GLIBMM_CFLAGS} -I${GLIB_INCLUDE_DIR})
    set(ENV{GLIBMM_LIBS} "${GLIB_LIBRARY}")
    set(ENV{GIOMM_CFLAGS} -I${GLIB_INCLUDE_DIR})
    set(ENV{GIOMM_LIBS} "${GIO_LIBRARY}")
    set(ENV{PKG_CONFIG} "echo")

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
    execute_process(
        COMMAND "${CMAKE_SOURCE_DIR}/configure"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/config
        RESULT_VARIABLE res
    )
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "Configure failed.")
    endif()
endif()

file(GLOB GLIBMM_SOURCES glib/glibmm/*.cc)
add_library(glibmm ${GLIBMM_SOURCES})
target_compile_definitions(glibmm PRIVATE -DGLIBMM_BUILD)
target_include_directories(glibmm PUBLIC glib ${CMAKE_BINARY_DIR}/config/glib)

file(GLOB GIOMM_SOURCES gio/giomm/*.cc)
if(WIN32)
    list(FILTER GIOMM_SOURCES EXCLUDE REGEX "desktopappinfo.cc\$|/unix[^/]+.cc\$")
endif()
add_library(giomm ${GIOMM_SOURCES})
target_compile_definitions(giomm PRIVATE -DGIOMM_BUILD)
target_link_libraries(giomm PUBLIC glibmm)
target_include_directories(giomm PUBLIC gio ${CMAKE_BINARY_DIR}/config/gio)

install(
    TARGETS glibmm giomm
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if(NOT DISABLE_INSTALL_HEADERS)
    install(
        FILES
            ${CMAKE_BINARY_DIR}/config/gio/giommconfig.h
            gio/giomm.h
            ${CMAKE_BINARY_DIR}/config/glib/glibmmconfig.h
            glib/glibmm.h
        DESTINATION include
    )
    install(
        DIRECTORY gio/giomm glib/glibmm
        DESTINATION include
        FILES_MATCHING PATTERN *.h
    )
endif()

if(NOT DISABLE_EXAMPLES)
    add_executable(options examples/options/main.cc)
    target_link_libraries(options PRIVATE giomm)
endif()
