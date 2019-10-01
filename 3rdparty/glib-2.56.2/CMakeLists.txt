cmake_minimum_required(VERSION 3.12.1)
project(glib C)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(GLIB_DLL_SUFFIX 2)
set(GLIB_LIB_SUFFIX 2.0)

file(REMOVE_RECURSE ${CMAKE_CURRENT_LIST_DIR}/glib/pcre)
file(WRITE ${CMAKE_CURRENT_LIST_DIR}/glib/pcre/Makefile.in)
file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/glib/win_iconv.c)

set(GLIB_VERSION 2.52.3)
set(BUILD_SHARED_LIBS ON)
add_compile_options("/utf-8")
add_compile_definitions($<$<CONFIG:Debug>:G_ENABLE_DEBUG>)
add_compile_definitions($<$<BOOL:BUILD_SHARED_LIBS>:DLL_EXPORT>)
set(CMAKE_DEBUG_POSTFIX d)

if(MSVC)
    add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
    add_definitions(/FI"msvc_recommended_pragmas.h")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# find dependencies
get_filename_component(zlibAbsInstallPath "../../install/zlib" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pcreAbsInstallPath "../../install/PCRE" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(iconvAbsInstallPath "../../install/libiconv" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(ffiAbsInstallPath "../../install/libffi" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(intlAbsInstallPath "../../install/gettext" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(localModuleAbsPath "../../cmake" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH ${localModuleAbsPath})
set(ZLIB_ROOT "${zlibAbsInstallPath}")
set(PCRE_ROOT "${pcreAbsInstallPath}")
set(Iconv_ROOT "${iconvAbsInstallPath}")
set(FFI_ROOT "${ffiAbsInstallPath}")
set(Intl_ROOT "${intlAbsInstallPath}")

# zlib
find_package(ZLIB REQUIRED)
# pcre
find_package(PCRE REQUIRED)
# libiconv
find_package(Iconv REQUIRED)
# libffi
find_package(FFI REQUIRED)
get_filename_component(LIB_DIR "${FFI_LIBRARY}" DIRECTORY)
if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(CORESERVICES_LIBRARY CoreServices)
    link_libraries(${CORESERVICES_LIBRARY} ${COREFOUNDATION_LIBRARY} ${FOUNDATION_LIBRARY})
endif()

if(WIN32 OR APPLE)
    # libintl(gettext)
    find_package(Intl REQUIRED)
    set(LIBINTL_INCLUDE_DIR ${Intl_INCLUDE_DIR})
    set(LIBINTL_LIBRARY ${Intl_LIBRARY})
else()
    set(LIBINTL_INCLUDE_DIR)
    set(LIBINTL_LIBRARY)
endif()

if(WIN32)
    set(LIBRESOLV_LIBRARY)
else()
    find_library(LIBRESOLV_LIBRARY NAMES resolv libresolv)
endif()

#prepare config files
if(WIN32)
    configure_file(config.h.win32 ${CMAKE_BINARY_DIR}/config/config.h COPYONLY)
    configure_file(glib/glibconfig.h.win32 ${CMAKE_BINARY_DIR}/config/glib/glibconfig.h COPYONLY)
    configure_file(gmodule/gmoduleconf.h.win32 ${CMAKE_BINARY_DIR}/config/gmodule/gmoduleconf.h COPYONLY)
    configure_file(gio/gnetworking.h.win32 ${CMAKE_BINARY_DIR}/config/gio/gnetworking.h COPYONLY)
    add_definitions(-DHAVE_CONFIG_H)
else()
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
    set(ENV{LIBFFI_LIBS} "${FFI_LIBRARY}")
    set(ENV{PCRE_LIBS} "${PCRE_LIBRARY}")
    set(ENV{LIBFFI_CFLAGS} "-I${FFI_INCLUDE_DIR}")
    set(ENV{PCRE_CFLAGS} "-I${PCRE_INCLUDE_DIR}")
    set(ENV{MSGFMT} "/bin/echo")
    set(ENV{GMSGFMT} "/bin/echo")
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/config/config.h")
        execute_process(
            COMMAND "${CMAKE_SOURCE_DIR}/configure"
                --disable-libelf
                --disable-libmount
                "CPPFLAGS=-I${PCRE_INCLUDE_DIR}"
                "CFLAGS=-I${PCRE_INCLUDE_DIR}"
                "LDFLAGS=-L${LIB_DIR}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/config
            RESULT_VARIABLE res
        )
        if(NOT res EQUAL 0)
            message(FATAL_ERROR "Configure failed.")
        endif()
    endif()
    add_definitions("-DGIO_MODULE_DIR=\"gio/modules\"")
endif()

include_directories(${CMAKE_BINARY_DIR}/config ${CMAKE_BINARY_DIR}/config/glib ${CMAKE_BINARY_DIR}/config/gio ${CMAKE_BINARY_DIR}/config/gmodule)
include_directories(. glib)

# This macro purposely doesn't find nodes with sources that have additional properties set
# Most of such files in glib are PCRE sources which we don't use anyway
macro(extract_vcproj_sources VC_PROJECT OUT_VAR)
    file(READ ${VC_PROJECT} ${VC_PROJECT}-CONTENTS)
    STRING(REPLACE "\n" ";" ${VC_PROJECT}-CONTENTS "${${VC_PROJECT}-CONTENTS}") # split by lines
    foreach(LINE ${${VC_PROJECT}-CONTENTS})
        if(LINE MATCHES "<ClCompile Include=\\\".*\\\" />")
            string(REPLACE "<ClCompile Include=\"..\\..\\..\\" "" LINE ${LINE})
            string(REPLACE "\" />" "" LINE ${LINE})
            string(STRIP ${LINE} LINE)
            file(TO_CMAKE_PATH ${LINE} LINE)
            list(APPEND ${OUT_VAR} ${LINE})
        endif()
    endforeach()
endmacro()

# main module
extract_vcproj_sources(win32/vs15/glib.vcxproj GLIB_SOURCES)
list(APPEND GLIB_SOURCES glib/libcharset/localcharset.c) # modified internal version with prefixed symbols
if(NOT WIN32)
    list(FILTER GLIB_SOURCES EXCLUDE REGEX "win32.c\$|win32-helper.c\$")
    list(APPEND GLIB_SOURCES "glib/gthread-posix.c" "glib/giounix.c" "glib/gspawn.c" "glib/glib-unix.c")
endif()
add_library(glib ${GLIB_SOURCES})
target_compile_definitions(glib PRIVATE GLIB_COMPILATION G_LOG_DOMAIN="GLib" LIBDIR="")
target_link_libraries(glib PRIVATE PCRE::pcre Iconv::Iconv Iconv::charset ${LIBINTL_LIBRARY})
if(WIN32)
    target_compile_definitions(glib PRIVATE USE_SYSTEM_PCRE)
    target_link_libraries(glib PRIVATE ws2_32 winmm)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(glib PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()
target_include_directories(glib PRIVATE ${PCRE_INCLUDE_DIR} ${ICONV_INCLUDE_DIR})
target_include_directories(glib PUBLIC $<BUILD_INTERFACE:${LIBINTL_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>)
list(APPEND GLIB_TARGETS glib)
if(NOT GLIB_SKIP_HEADERS)
    install(FILES glib/glib.h glib/glib-object.h ${CMAKE_BINARY_DIR}/config/glib/glibconfig.h DESTINATION include)

    file(GLOB GLIB_HEADERS glib/*.h)
    list(FILTER GLIB_HEADERS EXCLUDE REGEX "/glib.h\$|/glib-object.h\$|private.h\$")
    install(FILES ${GLIB_HEADERS} DESTINATION include/glib)

    file(GLOB GLIB_DEP_HEADERS glib/deprecated/*.h)
    install(FILES ${GLIB_DEP_HEADERS} DESTINATION include/glib/deprecated)
endif()

# gthread
add_library(gthread gthread/gthread-impl.c)
target_compile_definitions(gthread PRIVATE G_LOG_DOMAIN="GThread")
target_link_libraries(gthread PRIVATE glib ${LIBINTL_LIBRARY})
target_include_directories(gthread PRIVATE ${LIBINTL_INCLUDE_DIR})
list(APPEND GLIB_TARGETS gthread)

# gobject
extract_vcproj_sources(win32/vs15/gobject.vcxproj GOBJECT_SOURCES)
add_library(gobject ${GOBJECT_SOURCES})
target_compile_definitions(gobject PRIVATE GOBJECT_COMPILATION G_LOG_DOMAIN="GLib-GObject")
target_link_libraries(gobject PRIVATE gthread glib FFI::FFI)
target_include_directories(gobject PRIVATE PUBLIC $<INSTALL_INTERFACE:include>)
list(APPEND GLIB_TARGETS gobject)
if(NOT GLIB_SKIP_HEADERS)
    file(GLOB GOBJECT_HEADERS gobject/*.h gobject/gobjectnotifyqueue.c)
    list(FILTER GOBJECT_HEADERS EXCLUDE REGEX "private.h\$")
    install(FILES ${GOBJECT_HEADERS} DESTINATION include/gobject)
endif()

# gmodule
add_library(gmodule gmodule/gmodule.c)
target_compile_definitions(gmodule PRIVATE G_LOG_DOMAIN="GModule")
target_link_libraries(gmodule PRIVATE glib ${LIBINTL_LIBRARY})
target_include_directories(gmodule PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/gmodule>)
target_include_directories(gmodule PRIVATE ${LIBINTL_INCLUDE_DIR} PUBLIC $<INSTALL_INTERFACE:include>)
list(APPEND GLIB_TARGETS gmodule)
if(NOT GLIB_SKIP_HEADERS)
    install(FILES gmodule/gmodule.h DESTINATION include)
endif()

# gio subdirs
if(NOT WIN32)
    file(GLOB XDGMIME_SOURCES gio/xdgmime/*.c)
    add_library(xdgmime ${XDGMIME_SOURCES})
    target_compile_definitions(xdgmime PRIVATE -DXDG_PREFIX=_gio_xdg)
    list(APPEND GLIB_TARGETS xdgmime)
endif()

if(NOT WIN32 AND NOT APPLE)
    file(GLOB INOTIFY_SOURCES gio/inotify/*.c)
    add_library(inotify ${INOTIFY_SOURCES})
    target_link_libraries(inotify PRIVATE gmodule)
    target_compile_definitions(inotify PRIVATE -DG_LOG_DOMAIN=\"GLib-GIO\" -DGIO_COMPILATION -DG_DISABLE_DEPRECATED)
    list(APPEND GLIB_TARGETS inotify)
endif()

if(APPLE)
    file(GLOB KQUEUE_SOURCES gio/kqueue/*.c)
    add_library(kqueue ${KQUEUE_SOURCES})
    target_link_libraries(kqueue PRIVATE gmodule)
    target_compile_definitions(kqueue PRIVATE -DG_LOG_DOMAIN=\"GLib-GIO\" -DGIO_COMPILATION -DG_DISABLE_DEPRECATED)
    list(APPEND GLIB_TARGETS kqueue)
endif()

# gio
extract_vcproj_sources(win32/vs15/gio.vcxproj GIO_SOURCES)
if(NOT WIN32)
    file(GLOB GIO_UNIX_SOURCES "gio/gunix*.c" "gio/g*notificationbackend.c" "gio/g*portal*.c")
    list(APPEND GIO_SOURCES ${GIO_UNIX_SOURCES})
    list(APPEND GIO_SOURCES
        "gio/gcontenttype.c"
        "gio/gfiledescriptorbased.c"
        "gio/gnetworkmonitornm.c"
        "gio/xdp-dbus.c"
    )
    list(FILTER GIO_SOURCES EXCLUDE REGEX "/gwin32[^/]+\$|win32/[^/]+\$|win32.c\$|gregistrysettingsbackend.c\$")
    if(APPLE)
        set_property(SOURCE
            gio/gcocoanotificationbackend.c
            gio/gosxappinfo.c
            gio/gnextstepsettingsbackend.c
            PROPERTY COMPILE_FLAGS "-x objective-c")
        list(APPEND GIO_SOURCES
            "gio/gnextstepsettingsbackend.c"
            "gio/gosxappinfo.c"
        )
    else()
        list(APPEND GIO_SOURCES
            "gio/gnetworkmonitornetlink.c"
            "gio/gdesktopappinfo.c"
        )
        list(FILTER GIO_SOURCES EXCLUDE REGEX "gcocoanotificationbackend.c\$")
    endif()
endif()
if(NOT GLIB_SKIP_HEADERS)
    file(GLOB GIO_HEADERS gio/*.h)
    list(FILTER GIO_HEADERS EXCLUDE REGEX "private.h\$")
    install(FILES ${GIO_HEADERS} ${CMAKE_BINARY_DIR}/config/gio/gnetworking.h DESTINATION include/gio)
endif()
add_library(gio ${GIO_SOURCES})
target_compile_definitions(gio PRIVATE GIO_COMPILATION G_LOG_DOMAIN="GLib-GIO")
target_link_libraries(gio PRIVATE glib gmodule gobject ZLIB::ZLIB ${LIBRESOLV_LIBRARY} ${LIBINTL_LIBRARY})
target_include_directories(gio PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/gio> $<INSTALL_INTERFACE:include/gio>)
if(WIN32)
    target_link_libraries(gio PRIVATE ws2_32 shlwapi dnsapi iphlpapi)
elseif(APPLE)
    target_link_libraries(gio PRIVATE xdgmime kqueue)
else()
    target_link_libraries(gio PRIVATE xdgmime inotify)
endif()
list(APPEND GLIB_TARGETS gio)

foreach(GTARGET ${GLIB_TARGETS})
    set_target_properties(${GTARGET} PROPERTIES
        OUTPUT_NAME ${GTARGET}-${GLIB_DLL_SUFFIX}
        ARCHIVE_OUTPUT_NAME ${GTARGET}-${GLIB_LIB_SUFFIX})
endforeach()

macro(add_glib_tool TOOL_NAME)
    add_executable(${TOOL_NAME} ${ARGN})
    target_link_libraries(${TOOL_NAME} glib ${LIBINTL_LIBRARY})
    target_compile_definitions(${TOOL_NAME} PRIVATE GLIB_COMPILATION)
    list(APPEND GLIB_TOOLS ${TOOL_NAME})
endmacro()

macro(add_gio_tool TOOL_NAME)
    add_executable(${TOOL_NAME} ${ARGN})
    target_link_libraries(${TOOL_NAME} PRIVATE glib gio gobject gmodule ${LIBINTL_LIBRARY})
    target_include_directories(${TOOL_NAME} PRIVATE gmodule gio)
    target_compile_definitions(${TOOL_NAME} PRIVATE GIO_COMPILATION)
    list(APPEND GLIB_TOOLS ${TOOL_NAME})
endmacro()

if(NOT GLIB_SKIP_TOOLS)
    configure_file(gobject/glib-mkenums.in ${CMAKE_SOURCE_DIR}/gobject/glib-mkenums @ONLY) # uses GLIB_VERSION
    install(FILES gobject/glib-mkenums DESTINATION tools/glib)

    configure_file(gio/gdbus-2.0/codegen/gdbus-codegen.in ${CMAKE_SOURCE_DIR}/gio/gdbus-2.0/codegen/gdbus-codegen COPYONLY)
    install(FILES gio/gdbus-2.0/codegen/gdbus-codegen DESTINATION tools/glib)
    file(GLOB CODEGEN_SOURCES gio/gdbus-2.0/codegen/*.py)
    install(FILES ${CODEGEN_SOURCES} DESTINATION tools/glib/codegen)

    add_gio_tool(gdbus gio/gdbus-tool.c)
    add_gio_tool(gio-querymodules gio/gio-querymodules.c gio/giomodule-priv.c)
    file(GLOB GIO_TOOL_SOURCES gio/gio-tool*.c)
    add_gio_tool(gio-tool ${GIO_TOOL_SOURCES})
    set_target_properties(gio-tool PROPERTIES OUTPUT_NAME gio)
    add_gio_tool(glib-compile-resources gio/glib-compile-resources.c gio/gvdb/gvdb-builder.c)
    add_gio_tool(glib-compile-schemas gio/glib-compile-schemas.c gio/gvdb/gvdb-builder.c)
    add_gio_tool(gresource gio/gresource-tool.c)
    add_gio_tool(gsettings gio/gsettings-tool.c)

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(WIN win32)
    else()
        set(WIN win64)
    endif()

    add_glib_tool(glib-genmarshal gobject/glib-genmarshal.c)
    if(WIN32)
        add_glib_tool(gspawn-${WIN}-helper WIN32 glib/gspawn-win32-helper.c)
        add_glib_tool(gspawn-${WIN}-helper-console glib/gspawn-win32-helper-console.c)
    endif()

    install(TARGETS ${GLIB_TOOLS} RUNTIME DESTINATION tools/glib)
endif()

install(
    TARGETS ${GLIB_TARGETS}
    EXPORT glib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(
    EXPORT glib
    NAMESPACE unofficial::glib::
    FILE unofficial-glib-targets.cmake
    DESTINATION share/unofficial-glib
)
configure_file(
    cmake/unofficial-glib-config.in.cmake
    cmake/unofficial-glib-config.cmake
    @ONLY
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/unofficial-glib-config.cmake
    DESTINATION share/unofficial-glib
)

message(STATUS "Link-time dependencies:")
message(STATUS "  " ${ZLIB_LIBRARIES})
message(STATUS "  " ${PCRE_LIBRARY})
message(STATUS "  " ${ICONV_LIBRARY})
message(STATUS "  " ${CHARSET_LIBRARY})
message(STATUS "  " ${FFI_LIBRARY})
message(STATUS "  " ${LIBINTL_LIBRARY})
