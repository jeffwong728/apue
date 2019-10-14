find_path(GLIBMM_INCLUDE_DIR NAMES glibmm.h PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES include NO_DEFAULT_PATH)

find_library(GLIBMM_glibmm_LIBRARY_RELEASE NAMES glibmm PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library(GLIBMM_glibmm_LIBRARY_DEBUG NAMES glibmm glibmmd PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)

find_library(GLIBMM_giomm_LIBRARY_RELEASE NAMES giomm PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library(GLIBMM_giomm_LIBRARY_DEBUG NAMES giomm giommd PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)

include(SelectLibraryConfigurations)
select_library_configurations(GLIBMM_glibmm)
select_library_configurations(GLIBMM_giomm)

mark_as_advanced(GLIBMM_INCLUDE_DIR)
mark_as_advanced(GLIBMM_glibmm_LIBRARY)
mark_as_advanced(GLIBMM_giomm_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLIBMM FOUND_VAR GLIBMM_FOUND REQUIRED_VARS GLIBMM_glibmm_LIBRARY GLIBMM_giomm_LIBRARY GLIBMM_INCLUDE_DIR)

include(SpamCommon)
if(GLIBMM_FOUND)
  set(GLIBMM_INCLUDE_DIRS "${GLIBMM_INCLUDE_DIR}")
  set(GLIBMM_LIBRARIES "${GLIBMM_glibmm_LIBRARY}" "${GLIBMM_giomm_LIBRARY}")
  spam_export_pack_component(GLIBMM glibmm)
  spam_export_pack_component(GLIBMM giomm)
  set(GLIBMM_LIBRARY ${GLIBMM_glibmm_LIBRARY})
  set(GLIBMM_LIBRARY_DEBUG ${GLIBMM_glibmm_LIBRARY_DEBUG})
  set(GLIBMM_LIBRARY_RELEASE ${GLIBMM_glibmm_LIBRARY_RELEASE})
  set(GIOMM_LIBRARY ${GLIBMM_giomm_LIBRARY})
  set(GIOMM_LIBRARY_DEBUG ${GLIBMM_giomm_LIBRARY_DEBUG})
  set(GIOMM_LIBRARY_RELEASE ${GLIBMM_giomm_LIBRARY_RELEASE})
endif()
