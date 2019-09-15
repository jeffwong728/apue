find_path(OpenBLAS_INCLUDE_DIR NAMES "cblas.h" DOC "OpenBLAS include directory")

find_library(OpenBLAS_LIBRARY_RELEASE NAMES libopenblas DOC "OpenBLAS library release (potentially the C library)")
find_library(OpenBLAS_LIBRARY_DEBUG NAMES libopenblas DOC "OpenBLAS library debug (potentially the C library)")

include(SelectLibraryConfigurations)
select_library_configurations(OpenBLAS)

mark_as_advanced(OpenBLAS_INCLUDE_DIR)
mark_as_advanced(OpenBLAS_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenBLAS FOUND_VAR OpenBLAS_FOUND REQUIRED_VARS OpenBLAS_LIBRARY OpenBLAS_INCLUDE_DIR)

include(SpamCommon)
if(OpenBLAS_FOUND)
  set(OpenBLAS_INCLUDE_DIRS "${OpenBLAS_INCLUDE_DIR}")
  set(OpenBLAS_LIBRARIES "${OpenBLAS_LIBRARY}")
  spam_export_pack(OpenBLAS)
  #set_target_properties(FFI::FFI PROPERTIES INTERFACE_COMPILE_DEFINITIONS FFI_BUILDING)
endif()
