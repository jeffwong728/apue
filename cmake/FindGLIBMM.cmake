find_path(GLIBMM_INCLUDE_DIR NAMES "glibmm.h" DOC "Glibmm include directory")

find_library(GLIBMM_glibmm_LIBRARY_RELEASE NAMES "glibmm" DOC "glibmm library release")
find_library(GLIBMM_glibmm_LIBRARY_DEBUG NAMES "glibmmd" DOC "glibmm library debug")

find_library(GLIBMM_giomm_LIBRARY_RELEASE NAMES "giomm" DOC "giomm library release")
find_library(GLIBMM_giomm_LIBRARY_DEBUG NAMES "giommd" DOC "giomm library debug")

include(SelectLibraryConfigurations)
select_library_configurations(GLIBMM_glibmm)
select_library_configurations(GLIBMM_giomm)

mark_as_advanced(GLIBMM_INCLUDE_DIR)
mark_as_advanced(GLIBMM_glibmm_LIBRARY)
mark_as_advanced(GLIBMM_giomm_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLIBMM FOUND_VAR GLIBMM_FOUND REQUIRED_VARS GLIBMM_glibmm_LIBRARY GLIBMM_giomm_LIBRARY GLIBMM_INCLUDE_DIR)

include(SpamCommon)
if(GLIB_FOUND)
  set(GLIBMM_INCLUDE_DIRS "${GLIBMM_INCLUDE_DIR}")
  set(GLIBMM_LIBRARIES "${GLIBMM_glibmm_LIBRARY}" "${GLIBMM_giomm_LIBRARY}")
  spam_export_pack_component(GLIBMM glibmm)
  spam_export_pack_component(GLIBMM giomm)
  #set_target_properties(FFI::FFI PROPERTIES INTERFACE_COMPILE_DEFINITIONS FFI_BUILDING)
  set(GLIBMM_LIBRARY ${GLIBMM_glibmm_LIBRARY})
  set(GLIBMM_LIBRARY_DEBUG ${GLIBMM_glibmm_LIBRARY_DEBUG})
  set(GLIBMM_LIBRARY_RELEASE ${GLIBMM_glibmm_LIBRARY_RELEASE})
  set(GIOMM_LIBRARY ${GLIBMM_giomm_LIBRARY})
  set(GIOMM_LIBRARY_DEBUG ${GLIBMM_giomm_LIBRARY_DEBUG})
  set(GIOMM_LIBRARY_RELEASE ${GLIBMM_giomm_LIBRARY_RELEASE})
endif()
