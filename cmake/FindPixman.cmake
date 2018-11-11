# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindPixman
# --------
#
# Find the native Pixman includes and library.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``Pixman::Pixman``, if
# Pixman has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   Pixman_INCLUDE_DIRS   - where to find pixman.h, etc.
#   Pixman_LIBRARIES      - List of libraries when using pixman.
#   Pixman_FOUND          - True if pixman found.
#
# ::
#
#   Pixman_VERSION_STRING - The version of pixman found (x.y.z)
#   Pixman_VERSION_MAJOR  - The major version of pixman
#   Pixman_VERSION_MINOR  - The minor version of pixman
#   Pixman_VERSION_PATCH  - The patch version of pixman
#   Pixman_VERSION_TWEAK  - The tweak version of pixman
#
# Backward Compatibility
# ^^^^^^^^^^^^^^^^^^^^^^
#
# The following variable are provided for backward compatibility
#
# ::
#
#   Pixman_MAJOR_VERSION  - The major version of pixman
#   Pixman_MINOR_VERSION  - The minor version of pixman
#   Pixman_PATCH_VERSION  - The patch version of pixman
#
# Hints
# ^^^^^
#
# A user may set ``Pixman_ROOT`` to a pixman installation root to tell this
# module where to look.

set(_Pixman_SEARCHES)

# Search Pixman_ROOT first if it is set.
if(Pixman_ROOT)
  set(_Pixman_SEARCH_ROOT PATHS ${Pixman_ROOT} NO_DEFAULT_PATH)
  list(APPEND _Pixman_SEARCHES _Pixman_SEARCH_ROOT)
endif()

# Normal search.
set(_Pixman_SEARCH_NORMAL
  PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\Pixman;InstallPath]"
        "$ENV{PROGRAMFILES}/pixman"
  )
list(APPEND _Pixman_SEARCHES _Pixman_SEARCH_NORMAL)

set(Pixman_NAMES pixman-1)
set(Pixman_NAMES_DEBUG pixman-1d)

# Try each search configuration.
foreach(search ${_Pixman_SEARCHES})
  find_path(Pixman_INCLUDE_DIR NAMES pixman.h ${${search}} PATH_SUFFIXES include)
endforeach()

# Allow Pixman_LIBRARY to be set manually, as the location of the pixman library
if(NOT Pixman_LIBRARY)
  foreach(search ${_Pixman_SEARCHES})
    find_library(Pixman_LIBRARY_RELEASE NAMES ${Pixman_NAMES} NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
    find_library(Pixman_LIBRARY_DEBUG NAMES ${Pixman_NAMES_DEBUG} NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
  endforeach()

  include(SelectLibraryConfigurations)
  select_library_configurations(Pixman)
endif()

unset(Pixman_NAMES)
unset(Pixman_NAMES_DEBUG)

mark_as_advanced(Pixman_INCLUDE_DIR)

if(Pixman_INCLUDE_DIR AND EXISTS "${Pixman_INCLUDE_DIR}/pixman-version.h")
    file(STRINGS "${Pixman_INCLUDE_DIR}/pixman-version.h" Pixman_H REGEX "^#define Pixman_VERSION_STRING \"[^\"]*\"$")

    string(REGEX REPLACE "^.*Pixman_VERSION_STRING \"([0-9]+).*$" "\\1" Pixman_VERSION_MAJOR "${Pixman_H}")
    string(REGEX REPLACE "^.*Pixman_VERSION_STRING \"[0-9]+\\.([0-9]+).*$" "\\1" Pixman_VERSION_MINOR  "${Pixman_H}")
    string(REGEX REPLACE "^.*Pixman_VERSION_STRING \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" Pixman_VERSION_PATCH "${Pixman_H}")
    set(Pixman_VERSION_STRING "${Pixman_VERSION_MAJOR}.${Pixman_VERSION_MINOR}.${Pixman_VERSION_PATCH}")

    # only append a TWEAK version if it exists:
    set(Pixman_VERSION_TWEAK "")
    if( "${Pixman_H}" MATCHES "Pixman_VERSION_STRING \"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+)")
        set(Pixman_VERSION_TWEAK "${CMAKE_MATCH_1}")
        string(APPEND Pixman_VERSION_STRING ".${Pixman_VERSION_TWEAK}")
    endif()

    set(Pixman_MAJOR_VERSION "${Pixman_VERSION_MAJOR}")
    set(Pixman_MINOR_VERSION "${Pixman_VERSION_MINOR}")
    set(Pixman_PATCH_VERSION "${Pixman_VERSION_PATCH}")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Pixman REQUIRED_VARS Pixman_LIBRARY Pixman_INCLUDE_DIR
                                       VERSION_VAR Pixman_VERSION_STRING)

if(Pixman_FOUND)
    set(Pixman_INCLUDE_DIRS ${Pixman_INCLUDE_DIR})

    if(NOT Pixman_LIBRARIES)
      set(Pixman_LIBRARIES ${Pixman_LIBRARY})
    endif()

    if(NOT TARGET Pixman::Pixman)
      add_library(Pixman::Pixman UNKNOWN IMPORTED)
      set_target_properties(Pixman::Pixman PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Pixman_INCLUDE_DIRS}")

      if(Pixman_LIBRARY_RELEASE)
        set_property(TARGET Pixman::Pixman APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(Pixman::Pixman PROPERTIES
          IMPORTED_LOCATION_RELEASE "${Pixman_LIBRARY_RELEASE}")
      endif()

      if(Pixman_LIBRARY_DEBUG)
        set_property(TARGET Pixman::Pixman APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(Pixman::Pixman PROPERTIES
          IMPORTED_LOCATION_DEBUG "${Pixman_LIBRARY_DEBUG}")
      endif()

      if(NOT Pixman_LIBRARY_RELEASE AND NOT Pixman_LIBRARY_DEBUG)
        set_property(TARGET Pixman::Pixman APPEND PROPERTY
          IMPORTED_LOCATION "${Pixman_LIBRARY}")
      endif()
    endif()
endif()
