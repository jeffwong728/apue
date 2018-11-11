find_path(FontConfig_INCLUDE_DIR NAMES "fontconfig/fontconfig.h" DOC "fontconfig include directory")

find_library(FontConfig_LIBRARY_RELEASE NAMES fontconfig DOC "fontconfig library release (potentially the C library)")
find_library(FontConfig_LIBRARY_DEBUG NAMES fontconfigd DOC "fontconfig library debug (potentially the C library)")

include(SelectLibraryConfigurations)
select_library_configurations(FontConfig)

mark_as_advanced(FontConfig_INCLUDE_DIR)
mark_as_advanced(FontConfig_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FontConfig FOUND_VAR FontConfig_FOUND REQUIRED_VARS FontConfig_LIBRARY FontConfig_INCLUDE_DIR)

include(SpamCommon)
if(FontConfig_FOUND)
  set(FontConfig_INCLUDE_DIRS "${FontConfig_INCLUDE_DIR}")
  set(FontConfig_LIBRARIES "${FontConfig_LIBRARY}")
  spam_export_pack(FontConfig)
  #set_target_properties(FFI::FFI PROPERTIES INTERFACE_COMPILE_DEFINITIONS FFI_BUILDING)
endif()
