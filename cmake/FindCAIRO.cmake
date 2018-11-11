find_path(CAIRO_INCLUDE_DIR NAMES "cairo.h" DOC "Cairo include directory")

find_library(CAIRO_cairo_LIBRARY_RELEASE NAMES "cairo" DOC "cairo library release")
find_library(CAIRO_cairo_LIBRARY_DEBUG NAMES "cairod" DOC "cairo library debug")

find_library(CAIRO_gobject_LIBRARY_RELEASE NAMES "cairo-gobject" DOC "cairo gobject library release")
find_library(CAIRO_gobject_LIBRARY_DEBUG NAMES "cairo-gobjectd" DOC "cairo gobject library debug")

include(SelectLibraryConfigurations)
select_library_configurations(CAIRO_cairo)
select_library_configurations(CAIRO_gobject)

mark_as_advanced(CAIRO_INCLUDE_DIR)
mark_as_advanced(CAIRO_cairo_LIBRARY)
mark_as_advanced(CAIRO_gobject_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CAIRO FOUND_VAR CAIRO_FOUND REQUIRED_VARS CAIRO_cairo_LIBRARY CAIRO_gobject_LIBRARY CAIRO_INCLUDE_DIR)

include(SpamCommon)
if(CAIRO_FOUND)
  set(CAIRO_INCLUDE_DIRS "${CAIRO_INCLUDE_DIR}")
  set(CAIRO_LIBRARIES "${CAIRO_cairo_LIBRARY}" "${CAIRO_gobject_LIBRARY}")
  spam_export_pack_component(CAIRO cairo)
  spam_export_pack_component(CAIRO gobject)
  set_target_properties(CAIRO::cairo PROPERTIES INTERFACE_COMPILE_DEFINITIONS CAIRO_WIN32_STATIC_BUILD=1)
  set_target_properties(CAIRO::gobject PROPERTIES INTERFACE_COMPILE_DEFINITIONS CAIRO_WIN32_STATIC_BUILD=1)
  set(CAIRO_LIBRARY ${CAIRO_cairo_LIBRARY})
  set(CAIRO_LIBRARY_DEBUG ${CAIRO_cairo_LIBRARY_DEBUG})
  set(CAIRO_LIBRARY_RELEASE ${CAIRO_cairo_LIBRARY_RELEASE})
endif()
