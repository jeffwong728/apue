find_path(FFI_INCLUDE_DIR NAMES "ffi.h" DOC "ffi include directory")

find_library(FFI_LIBRARY_RELEASE NAMES libffi DOC "ffi library release (potentially the C library)")
find_library(FFI_LIBRARY_DEBUG NAMES libffid DOC "ffi library debug (potentially the C library)")

include(SelectLibraryConfigurations)
select_library_configurations(FFI)

mark_as_advanced(FFI_INCLUDE_DIR)
mark_as_advanced(FFI_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFI FOUND_VAR FFI_FOUND REQUIRED_VARS FFI_LIBRARY FFI_INCLUDE_DIR)

include(SpamCommon)
if(FFI_FOUND)
  set(FFI_INCLUDE_DIRS "${FFI_INCLUDE_DIR}")
  set(FFI_LIBRARIES "${FFI_LIBRARY}")
  spam_export_pack(FFI)
  set_target_properties(FFI::FFI PROPERTIES INTERFACE_COMPILE_DEFINITIONS FFI_BUILDING)
endif()
