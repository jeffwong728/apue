find_path(SIMD_INCLUDE_DIR NAMES "Simd/SimdLib.hpp" DOC "Simd include directory")

find_library(SIMD_Simd_LIBRARY_RELEASE NAMES "Simd" DOC "Simd library release")
find_library(SIMD_Simd_LIBRARY_DEBUG NAMES "Simdd" DOC "Simd library debug")

include(SelectLibraryConfigurations)
select_library_configurations(SIMD_Simd)

mark_as_advanced(SIMD_INCLUDE_DIR)
mark_as_advanced(SIMD_Simd_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SIMD FOUND_VAR SIMD_FOUND REQUIRED_VARS SIMD_Simd_LIBRARY SIMD_INCLUDE_DIR)

include(SpamCommon)
if(SIMD_FOUND)
  set(SIMD_INCLUDE_DIRS "${SIMD_INCLUDE_DIR}")
  set(SIMD_LIBRARIES "${SIMD_Simd_LIBRARY}")
  spam_export_pack_component(SIMD Simd)
  #set(SIMD_LIBRARY ${SIMD_Simd_LIBRARY})
  #set(SIMD_LIBRARY_DEBUG ${SIMD_Simd_LIBRARY_DEBUG})
  #set(SIMD_LIBRARY_RELEASE ${SIMD_Simd_LIBRARY_RELEASE})
endif()
