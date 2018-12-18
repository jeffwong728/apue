find_path(WXSVG_INCLUDE_DIR NAMES "wxSVG/SVGDocument.h" DOC "wxSVG include directory")

find_library(WXSVG_LIBRARY_RELEASE NAMES wxsvg DOC "wxSVG library release")
find_library(WXSVG_LIBRARY_DEBUG NAMES wxsvgd DOC "wxSVG library debug")

include(SelectLibraryConfigurations)
select_library_configurations(WXSVG)

mark_as_advanced(WXSVG_INCLUDE_DIR)
mark_as_advanced(WXSVG_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WXSVG FOUND_VAR WXSVG_FOUND REQUIRED_VARS WXSVG_LIBRARY WXSVG_INCLUDE_DIR)

include(SpamCommon)
if(WXSVG_FOUND)
  set(WXSVG_INCLUDE_DIRS "${WXSVG_INCLUDE_DIR}")
  set(WXSVG_LIBRARIES "${WXSVG_LIBRARY}")
  spam_export_pack(WXSVG)
endif()
