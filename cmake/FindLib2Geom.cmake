find_path(LIB2GEOM_INCLUDE_DIR NAMES 2geom/2geom.h PATH_SUFFIXES include DOC "lib2geom include directory")

find_library(LIB2GEOM_2geom_LIBRARY_RELEASE NAMES "2geom" DOC "lib2geom library release")
find_library(LIB2GEOM_2geom_LIBRARY_DEBUG NAMES "2geomd" DOC "lib2geom library debug")

include(SelectLibraryConfigurations)
select_library_configurations(LIB2GEOM_2geom)

mark_as_advanced(LIB2GEOM_INCLUDE_DIR)
mark_as_advanced(LIB2GEOM_2geom_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIB2GEOM FOUND_VAR LIB2GEOM_FOUND REQUIRED_VARS LIB2GEOM_2geom_LIBRARY LIB2GEOM_INCLUDE_DIR)

include(SpamCommon)
if(LIB2GEOM_FOUND)
  set(LIB2GEOM_INCLUDE_DIRS "${LIB2GEOM_INCLUDE_DIR}")
  set(LIB2GEOM_LIBRARIES "${LIB2GEOM_2geom_LIBRARY}")
  spam_export_pack_component(LIB2GEOM 2geom)
endif()