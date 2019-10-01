cmake_minimum_required(VERSION 3.12)
project(cairo C)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(BUILD_SHARED_LIBS OFF)
add_compile_options("/utf-8")
add_compile_definitions($<$<NOT:$<BOOL:BUILD_SHARED_LIBS>>:CAIRO_WIN32_STATIC_BUILD=1>)
add_compile_definitions(CAIRO_WIN32_STATIC_BUILD=1)

# Add include directories
include_directories(".")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  include_directories("./win32")
endif()

file(GLOB SOURCES
"cairo-analysis-surface.c"
"cairo-arc.c"
"cairo-array.c"
"cairo-atomic.c"
"cairo-base64-stream.c"
"cairo-base85-stream.c"
"cairo-bentley-ottmann.c"
"cairo-bentley-ottmann-rectangular.c"
"cairo-bentley-ottmann-rectilinear.c"
"cairo-botor-scan-converter.c"
"cairo-boxes.c"
"cairo-boxes-intersect.c"
"cairo.c"
"cairo-cache.c"
"cairo-clip.c"
"cairo-clip-boxes.c"
"cairo-clip-polygon.c"
"cairo-clip-region.c"
"cairo-clip-surface.c"
"cairo-color.c"
"cairo-composite-rectangles.c"
"cairo-compositor.c"
"cairo-contour.c"
"cairo-damage.c"
"cairo-debug.c"
"cairo-default-context.c"
"cairo-device.c"
"cairo-error.c"
"cairo-fallback-compositor.c"
"cairo-fixed.c"
"cairo-font-face.c"
"cairo-font-face-twin.c"
"cairo-font-face-twin-data.c"
"cairo-font-options.c"
"cairo-freelist.c"
"cairo-freed-pool.c"
"cairo-gstate.c"
"cairo-hash.c"
"cairo-hull.c"
"cairo-image-compositor.c"
"cairo-image-info.c"
"cairo-image-source.c"
"cairo-image-surface.c"
"cairo-line.c"
"cairo-lzw.c"
"cairo-matrix.c"
"cairo-mask-compositor.c"
"cairo-mesh-pattern-rasterizer.c"
"cairo-mempool.c"
"cairo-misc.c"
"cairo-mono-scan-converter.c"
"cairo-mutex.c"
"cairo-no-compositor.c"
"cairo-observer.c"
"cairo-output-stream.c"
"cairo-paginated-surface.c"
"cairo-path-bounds.c"
"cairo-path.c"
"cairo-path-fill.c"
"cairo-path-fixed.c"
"cairo-path-in-fill.c"
"cairo-path-stroke.c"
"cairo-path-stroke-boxes.c"
"cairo-path-stroke-polygon.c"
"cairo-path-stroke-traps.c"
"cairo-path-stroke-tristrip.c"
"cairo-pattern.c"
"cairo-pen.c"
"cairo-polygon.c"
"cairo-polygon-intersect.c"
"cairo-polygon-reduce.c"
"cairo-raster-source-pattern.c"
"cairo-recording-surface.c"
"cairo-rectangle.c"
"cairo-rectangular-scan-converter.c"
"cairo-region.c"
"cairo-rtree.c"
"cairo-scaled-font.c"
"cairo-shape-mask-compositor.c"
"cairo-slope.c"
"cairo-spans.c"
"cairo-spans-compositor.c"
"cairo-spline.c"
"cairo-stroke-dash.c"
"cairo-stroke-style.c"
"cairo-surface.c"
"cairo-surface-clipper.c"
"cairo-surface-fallback.c"
"cairo-surface-observer.c"
"cairo-surface-offset.c"
"cairo-surface-snapshot.c"
"cairo-surface-subsurface.c"
"cairo-surface-wrapper.c"
"cairo-time.c"
"cairo-tor-scan-converter.c"
"cairo-tor22-scan-converter.c"
"cairo-clip-tor-scan-converter.c"
"cairo-tag-attributes.c"
"cairo-tag-stack.c"
"cairo-toy-font-face.c"
"cairo-traps.c"
"cairo-tristrip.c"
"cairo-traps-compositor.c"
"cairo-unicode.c"
"cairo-user-font.c"
"cairo-version.c"
"cairo-wideint.c"
# generic font support
"cairo-cff-subset.c"
"cairo-scaled-font-subsets.c"
"cairo-truetype-subset.c"
"cairo-type1-fallback.c"
"cairo-type1-glyph-names.c"
"cairo-type1-subset.c"
"cairo-type3-glyph-surface.c"
# pdf
"cairo-pdf-interchange.c"
"cairo-pdf-operators.c"
"cairo-pdf-shading.c"
"cairo-pdf-surface.c"
# png
"cairo-png.c"
# ps surface
"cairo-ps-surface.c"
# deflate source
"cairo-deflate-stream.c"
# svg surface
"cairo-svg-surface.c"
# script surface
"cairo-script-surface.c"
# fontconfig + freetype
"cairo-ft-font.c"
)

# win32
file(GLOB PLATFORM_SOURCES_WIN32
"win32/cairo-win32-debug.c"
"win32/cairo-win32-device.c"
"win32/cairo-win32-gdi-compositor.c"
"win32/cairo-win32-system.c"
"win32/cairo-win32-surface.c"
"win32/cairo-win32-display-surface.c"
"win32/cairo-win32-printing-surface.c"
"win32/cairo-win32-font.c"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND SOURCES ${PLATFORM_SOURCES_WIN32})
endif()

set(CMAKE_DEBUG_POSTFIX "d")

get_filename_component(zlibAbsInstallPath "../../../install/zlib" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pngAbsInstallPath "../../../install/libpng" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(pixmanAbsInstallPath "../../../install/pixman" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(freetypeAbsInstallPath "../../../install/freetype" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(fontconfigAbsInstallPath "../../../install/fontconfig" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(glibAbsInstallPath "../../../install/glib" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(localModuleAbsPath "../../../cmake" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH ${localModuleAbsPath})
set(ZLIB_ROOT "${zlibAbsInstallPath}")
set(PNG_ROOT "${pngAbsInstallPath}")
set(Pixman_ROOT "${pixmanAbsInstallPath}")
set(Freetype_ROOT "${freetypeAbsInstallPath}")
set(FontConfig_ROOT "${fontconfigAbsInstallPath}")
set(GLIB_ROOT "${glibAbsInstallPath}")

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Pixman REQUIRED)
find_package(Freetype REQUIRED)

# Cairo needs to be told which features of FreeType are availible
add_definitions(
    -DHAVE_FT_GLYPHSLOT_EMBOLDEN=1
    -DHAVE_FT_LIBRARY_SETLCDFILTER=1
    -DHAVE_FT_GLYPHSLOT_OBLIQUE=1
    -DHAVE_FT_LOAD_SFNT_TABLE=1
    -DHAVE_FT_GET_X11_FONT_FORMAT=1)

# Find FontConfig
find_package(FontConfig REQUIRED)

# additional features for macOS
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_definitions(
      -DHAVE_INTTYPES_H=1
      -DHAVE_STDINT_H=1
      -DHAVE_SYS_TYPES_H=1
      -DHAVE_UINT64_T=1
      -DHAVE_UNISTD_H=1
      -DCAIRO_HAS_PTHREAD=1
      -DCAIRO_HAS_REAL_PTHREAD=1)
endif()

add_library(cairo ${SOURCES})

if(MSVC)
  # cairo produces a lot of warnings which are disabled here because they otherwise fill up the log files
  # NOTE: options only available to MSVC, clang in macOS doesn't understand these flags
  target_compile_options(cairo PUBLIC "/wd4244" PUBLIC "/wd4146" PUBLIC "/wd4312" PUBLIC "/wd4267" 
                               PUBLIC "/wd4996" PUBLIC "/wd4311" PUBLIC "/wd4334" PUBLIC "/wd4101")
endif()

target_link_libraries(cairo gdi32 msimg32 user32 ZLIB::ZLIB PNG::PNG Freetype::Freetype Pixman::Pixman FontConfig::FontConfig)

install(TARGETS cairo
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# GObject support module

set(CAIRO_GOBJECT_SOURCES
    "../util/cairo-gobject/cairo-gobject-enums.c"
    "../util/cairo-gobject/cairo-gobject-structs.c")

# GObject support sources do not include header with export macro
if(BUILD_SHARED_LIBS)
    set_source_files_properties(
        "../util/cairo-gobject/cairo-gobject-enums.c"
        "../util/cairo-gobject/cairo-gobject-structs.c"
        PROPERTIES COMPILE_DEFINITIONS "cairo_public=__declspec(dllexport)")
endif()

# Make GLib's GObject available
find_package(GLIB REQUIRED)

add_library(cairo-gobject ${CAIRO_GOBJECT_SOURCES}) 
target_link_libraries(cairo-gobject PUBLIC cairo PRIVATE GLIB::glib GLIB::gobject)

install(TARGETS cairo-gobject
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

set(pubHeaders
  "cairo.h"
  "cairo-deprecated.h"
  "cairo-features.h"
  "cairo-pdf.h"
  "cairo-ps.h"
  "cairo-script.h"
  "cairo-svg.h"
  "../cairo-version.h"
  "cairo-win32.h"
  "../util/cairo-gobject/cairo-gobject.h"
  "cairo-ft.h")

install(FILES ${pubHeaders} DESTINATION include)
install(FILES ${pubHeaders} DESTINATION include/cairo)
