find_path(EXIF_INCLUDE_DIR NAMES libexif/exif.h PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES include NO_DEFAULT_PATH)

find_library(EXIF_LIBRARY_RELEASE NAMES libexif PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library(EXIF_LIBRARY_DEBUG NAMES libexifd libexif PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)

include(SelectLibraryConfigurations)
select_library_configurations(EXIF)

mark_as_advanced(EXIF_INCLUDE_DIR)
mark_as_advanced(EXIF_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EXIF FOUND_VAR EXIF_FOUND REQUIRED_VARS EXIF_LIBRARY EXIF_INCLUDE_DIR)

include(SpamCommon)
if(EXIF_FOUND)
  set(EXIF_INCLUDE_DIRS "${EXIF_INCLUDE_DIR}")
  set(EXIF_LIBRARIES "${EXIF_LIBRARY}")
  spam_export_pack(EXIF)
endif()
