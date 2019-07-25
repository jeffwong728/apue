find_path(ASMLIB_INCLUDE_DIR NAMES "asmlib.h" DOC "asmlib include directory")

find_library(ASMLIB_LIBRARY_RELEASE NAMES libacof64 DOC "A multi-platform library of highly optimized functions for C and C++")
find_library(ASMLIB_LIBRARY_DEBUG NAMES libacof64 DOC "A multi-platform library of highly optimized functions for C and C++")

include(SelectLibraryConfigurations)
select_library_configurations(ASMLIB)

mark_as_advanced(ASMLIB_INCLUDE_DIR)
mark_as_advanced(ASMLIB_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASMLIB FOUND_VAR ASMLIB_FOUND REQUIRED_VARS ASMLIB_LIBRARY ASMLIB_INCLUDE_DIR)

include(SpamCommon)
if(ASMLIB_FOUND)
  set(ASMLIB_INCLUDE_DIRS "${ASMLIB_INCLUDE_DIR}")
  set(ASMLIB_LIBRARIES "${ASMLIB_LIBRARY}")
  spam_export_pack(ASMLIB)
endif()