cmake_minimum_required(VERSION 3.12.1)
project(pixman VERSION 0.32.0 LANGUAGES C)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
add_compile_options("/utf-8")
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_DEBUG_POSTFIX "d")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DTLS=__thread)
    add_definitions(-DHAVE_PTHREADS)
endif()

include_directories(".")
set(PIXMAN_PUBLIC_HDRS
    pixman.h
    pixman-accessor.h
    pixman-combine32.h
    pixman-compiler.h
    pixman-edge-imp.h
    pixman-inlines.h
    pixman-private.h
    pixman-version.h
)

file(GLOB SOURCES
"pixman-access-accessors.c"
"pixman-access.c"
"pixman-arm-neon.c"
"pixman-arm-simd.c"
"pixman-arm.c"
"pixman-bits-image.c"
"pixman-combine-float.c"
"pixman-combine32.c"
"pixman-conical-gradient.c"
"pixman-edge-accessors.c"
"pixman-edge.c"
"pixman-fast-path.c"
"pixman-filter.c"
"pixman-general.c"
"pixman-glyph.c"
"pixman-gradient-walker.c"
"pixman-image.c"
"pixman-implementation.c"
"pixman-linear-gradient.c"
"pixman-matrix.c"
"pixman-mips-dspr2.c"
"pixman-mips.c"
"pixman-mmx.c"
"pixman-noop.c"
"pixman-ppc.c"
"pixman-radial-gradient.c"
"pixman-region16.c"
"pixman-region32.c"
"pixman-solid-fill.c"
"pixman-sse2.c"
"pixman-ssse3.c"
"pixman-timer.c"
"pixman-trap.c"
"pixman-utils.c"
"pixman-x86.c"
"pixman.c"
)

add_library(pixman-1 STATIC ${SOURCES} ${PIXMAN_PUBLIC_HDRS})

# pixman requires the three PACKAGE* definitions in order to compile. The USE_SSE2 definition lets it use SSE2 instructions for speed. Every target machine should have SSE2 these days.
target_compile_definitions(pixman-1
    PUBLIC
        PACKAGE="pixman-1"
        PACKAGE_VERSION="0.34.0"
        PACKAGE_BUGREPORT=""
    PRIVATE
        USE_SSE2
        USE_SSSE3
)

# pixman produces a lot of warnings which are disabled here because they otherwise fill up the log files
if(MSVC)
    target_compile_options(pixman-1 PRIVATE "/wd4244" "/wd4146" "/wd4996") #  PUBLIC "/D_CRT_SECURE_NO_WARNINGS"
endif()

install(TARGETS pixman-1
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
install(FILES ${PIXMAN_PUBLIC_HDRS} DESTINATION "${INSTALL_INC_DIR}")