cmake_minimum_required(VERSION 3.12)
project(fontconfig C)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(FC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(CMAKE_DEBUG_POSTFIX "d")
add_compile_options("/utf-8")
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DHAVE_CONFIG_H)

get_filename_component(iconvAbsInstallPath "../../install/libiconv" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(ftAbsInstallPath "../../install/freetype" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(expatAbsInstallPath "../../install/expat" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(localModuleAbsPath "../../cmake" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH ${localModuleAbsPath})
set(Iconv_ROOT "${iconvAbsInstallPath}")
set(Freetype_ROOT "${ftAbsInstallPath}")
set(EXPAT_ROOT "${expatAbsInstallPath}")

# find dependencies
find_package(Iconv REQUIRED)
find_package(Freetype REQUIRED)
find_package(EXPAT REQUIRED)

add_library(fontconfig STATIC
    src/fcatomic.c
    src/fcblanks.c
    src/fccache.c
    src/fccfg.c
    src/fccharset.c
    src/fccompat.c
    src/fcdbg.c
    src/fcdefault.c
    src/fcdir.c
    src/fcformat.c
    src/fcfreetype.c
    src/fcfs.c
    src/fcinit.c
    src/fclang.c
    src/fclist.c
    src/fcmatch.c
    src/fcmatrix.c
    src/fcname.c
    src/fcobjs.c
    src/fcpat.c
    src/fcrange.c
    src/fcserialize.c
    src/fcstat.c
    src/fcstr.c
    src/fcweight.c
    src/fcxml.c
    src/ftglue.c)

set_target_properties(fontconfig PROPERTIES DEFINE_SYMBOL "FcPublic=__declspec(dllexport)")

target_include_directories(fontconfig PRIVATE .
    ${FC_INCLUDE_DIR}/alias
    ${EXPAT_INCLUDE_DIR})
if(WIN32)
    target_include_directories(fontconfig PRIVATE ${FC_INCLUDE_DIR}/win32)
    target_include_directories(fontconfig PRIVATE "../../install/dirent/include")
else()
    target_include_directories(fontconfig PRIVATE ${FC_INCLUDE_DIR}/unix)
endif()

target_link_libraries(fontconfig PRIVATE Iconv::Iconv Freetype::Freetype EXPAT::EXPAT)

install(TARGETS fontconfig
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

if(NOT FC_SKIP_HEADERS)
    install(FILES
        fontconfig/fontconfig.h
        fontconfig/fcfreetype.h
        fontconfig/fcprivate.h
        DESTINATION include/fontconfig)
endif()

if(NOT FC_SKIP_TOOLS)
    set(FC_DEFAULT_FONTS WINDOWSFONTDIR)
    set(FC_FONTPATH)
    set(FC_CACHEDIR WINDOWSTEMPDIR_FONTCONFIG_CACHE)
    set(CONFIGDIR ./fonts/conf.d)
    configure_file(fonts.conf.in ${CMAKE_SOURCE_DIR}/fonts.conf @ONLY)
    install(FILES fonts.conf DESTINATION tools/fontconfig/fonts)
    install(DIRECTORY conf.d DESTINATION tools/fontconfig/fonts FILES_MATCHING PATTERN "*.conf")
endif()
