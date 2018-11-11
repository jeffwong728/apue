# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindJPEGTurbo
# ---------
#
# Find the native JPEG-Turbo headers and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``JPEG::jpeg``
#   The JPEG ``jpeg`` library, if found.
#
# ``JPEG::turbo``
#   The JPEG ``turbo`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``JPEG_INCLUDE_DIRS``
#   where to find jpeglib.h, etc.
# ``JPEG_LIBRARIES``
#   the libraries to link against to use JPEG.
# ``JPEG_FOUND``
#   true if the JPEG headers and libraries were found.
#

set(_JPEG_SEARCHES)

# Search JPEG_ROOT first if it is set.
if(JPEG_ROOT)
  set(_JPEG_SEARCH_ROOT PATHS ${JPEG_ROOT} NO_DEFAULT_PATH)
  list(APPEND _JPEG_SEARCHES _JPEG_SEARCH_ROOT)
endif()

# Look for the header file.
foreach(search ${_JPEG_SEARCHES})
  find_path(JPEG_INCLUDE_DIR NAMES jpeglib.h ${${search}} PATH_SUFFIXES include)
endforeach()

include(SelectLibraryConfigurations)

# Look for jpeg library.
if(NOT JPEG_jpeg_LIBRARY)
  foreach(search ${_JPEG_SEARCHES})
    find_library(JPEG_jpeg_LIBRARY_RELEASE NAMES jpeg-static NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
    find_library(JPEG_jpeg_LIBRARY_DEBUG NAMES jpeg-staticd NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
  endforeach()
  select_library_configurations(JPEG_jpeg)
endif()

# Look for turbo library.
if(NOT JPEG_turbo_LIBRARY)
  foreach(search ${_JPEG_SEARCHES})
    find_library(JPEG_turbo_LIBRARY_RELEASE NAMES turbojpeg-static NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
    find_library(JPEG_turbo_LIBRARY_DEBUG NAMES turbojpeg-staticd NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
  endforeach()
  select_library_configurations(JPEG_turbo)
endif()

mark_as_advanced(JPEG_INCLUDE_DIR)

if(JPEG_INCLUDE_DIR AND EXISTS "${JPEG_INCLUDE_DIR}/jpeglib.h")
  file(STRINGS "${JPEG_INCLUDE_DIR}/jpeglib.h" jpeg_lib_version REGEX "^#define[\t ]+JPEG_LIB_VERSION[\t ]+.*")

  if (NOT jpeg_lib_version)
    # libjpeg-turbo sticks JPEG_LIB_VERSION in jconfig.h
      file(STRINGS "${JPEG_INCLUDE_DIR}/jconfig.h" jpeg_lib_version REGEX "^#define[\t ]+JPEG_LIB_VERSION[\t ]+.*")
  endif()

  string(REGEX REPLACE "^#define[\t ]+JPEG_LIB_VERSION[\t ]+([0-9]+).*"
    "\\1" JPEG_VERSION "${jpeg_lib_version}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JPEG
  FOUND_VAR
    JPEG_FOUND
  REQUIRED_VARS
    JPEG_INCLUDE_DIR
    JPEG_jpeg_LIBRARY
    JPEG_turbo_LIBRARY
  VERSION_VAR
    JPEG_VERSION)

mark_as_advanced( JPEG_ROOT )
mark_as_advanced( JPEG_VERSION )
mark_as_advanced( JPEG_jpeg_LIBRARY )
mark_as_advanced( JPEG_turbo_LIBRARY )

macro(export_JPEG_component comp_name)
  if(NOT TARGET JPEG::${comp_name})
      add_library(JPEG::${comp_name} UNKNOWN IMPORTED)
      set_target_properties(JPEG::${comp_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INCLUDE_DIRS}")
      #set_target_properties(JPEG::${comp_name} PROPERTIES INTERFACE_COMPILE_DEFINITIONS PCRE_STATIC)

      if(JPEG_${comp_name}_LIBRARY_RELEASE)
        set_property(TARGET JPEG::${comp_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(JPEG::${comp_name} PROPERTIES IMPORTED_LOCATION_RELEASE "${JPEG_${comp_name}_LIBRARY_RELEASE}")
      endif()

      if(JPEG_${comp_name}_LIBRARY_DEBUG)
        set_property(TARGET JPEG::${comp_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(JPEG::${comp_name} PROPERTIES IMPORTED_LOCATION_DEBUG "${JPEG_${comp_name}_LIBRARY_DEBUG}")
      endif()

      if(NOT JPEG_${comp_name}_LIBRARY_RELEASE AND NOT JPEG_${comp_name}_LIBRARY_DEBUG)
        set_property(TARGET JPEG::${comp_name} APPEND PROPERTY IMPORTED_LOCATION "${JPEG_${comp_name}_LIBRARY}")
      endif()
  endif()
endmacro(export_JPEG_component)

# Copy the results to the output variables and target.
if(JPEG_FOUND AND NOT TARGET JPEG::jpeg )
    set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
    if(NOT JPEG_LIBRARIES)
        set(JPEG_LIBRARIES ${JPEG_jpeg_LIBRARY} ${JPEG_turbo_LIBRARY} )
    endif()

    export_JPEG_component(jpeg)
    export_JPEG_component(turbo)

    set (JPEG_LIBRARY ${JPEG_jpeg_LIBRARY} CACHE FILEPATH "")
    set (JPEG_LIBRARY_DEBUG ${JPEG_jpeg_LIBRARY_DEBUG} CACHE FILEPATH "")
    set (JPEG_LIBRARY_RELEASE ${JPEG_jpeg_LIBRARY_RELEASE} CACHE FILEPATH "")
    mark_as_advanced( JPEG_LIBRARY )
    mark_as_advanced( JPEG_LIBRARY_DEBUG )
    mark_as_advanced( JPEG_LIBRARY_RELEASE )
endif()
