find_path(SKIA_INCLUDE_DIR NAMES "c/sk_canvas.h" DOC "Skia include directory")

find_library(SKIA_skia_LIBRARY_RELEASE NAMES "skia" DOC "Skia library release")
find_library(SKIA_skia_LIBRARY_DEBUG NAMES "skiad" DOC "Skia library debug")

find_library(SKIA_pathkit_LIBRARY_RELEASE NAMES "pathkit" DOC "Skia pathkit library release")
find_library(SKIA_pathkit_LIBRARY_DEBUG NAMES "pathkitd" DOC "Skia pathkit library debug")

include(SelectLibraryConfigurations)
select_library_configurations(SKIA_skia)
select_library_configurations(SKIA_pathkit)

mark_as_advanced(SKIA_INCLUDE_DIR)
mark_as_advanced(SKIA_skia_LIBRARY)
mark_as_advanced(SKIA_pathkit_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SKIA FOUND_VAR SKIA_FOUND REQUIRED_VARS SKIA_skia_LIBRARY SKIA_pathkit_LIBRARY SKIA_INCLUDE_DIR)

include(SpamCommon)
if(SKIA_FOUND)
  set(SKIA_INCLUDE_DIRS "${SKIA_INCLUDE_DIR}/core" "${SKIA_INCLUDE_DIR}/config" "${SKIA_INCLUDE_DIR}/gpu" "${SKIA_INCLUDE_DIR}/effects" "${SKIA_INCLUDE_DIR}/utils" "${SKIA_INCLUDE_DIR}/private" "${SKIA_INCLUDE_DIR}/pathops")
  set(SKIA_LIBRARIES "${SKIA_skia_LIBRARY}" "${SKIA_pathkit_LIBRARY}")
  spam_export_pack_component(SKIA skia)
  spam_export_pack_component(SKIA pathkit)
  #set_target_properties(CAIRO::cairo PROPERTIES INTERFACE_COMPILE_DEFINITIONS CAIRO_WIN32_STATIC_BUILD=1)
  #set_target_properties(CAIRO::gobject PROPERTIES INTERFACE_COMPILE_DEFINITIONS CAIRO_WIN32_STATIC_BUILD=1)
  set(SKIA_LIBRARY ${SKIA_skia_LIBRARY})
  set(SKIA_LIBRARY_DEBUG ${SKIA_skia_LIBRARY_DEBUG})
  set(SKIA_LIBRARY_RELEASE ${SKIA_skia_LIBRARY_RELEASE})
endif()
