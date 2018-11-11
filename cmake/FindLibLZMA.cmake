# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindLibLZMA
# -----------
#
# Find LibLZMA
#
# Find LibLZMA headers and library
#
# ::
#
#   LIBLZMA_FOUND             - True if liblzma is found.
#   LIBLZMA_INCLUDE_DIRS      - Directory where liblzma headers are located.
#   LIBLZMA_LIBRARIES         - Lzma libraries to link against.
#   LIBLZMA_HAS_AUTO_DECODER  - True if lzma_auto_decoder() is found (required).
#   LIBLZMA_HAS_EASY_ENCODER  - True if lzma_easy_encoder() is found (required).
#   LIBLZMA_HAS_LZMA_PRESET   - True if lzma_lzma_preset() is found (required).
#   LIBLZMA_VERSION_MAJOR     - The major version of lzma
#   LIBLZMA_VERSION_MINOR     - The minor version of lzma
#   LIBLZMA_VERSION_PATCH     - The patch version of lzma
#   LIBLZMA_VERSION_STRING    - version number as a string (ex: "5.0.3")

set(_LIBLZMA_SEARCHES)

# Search LIBLZMA_ROOT first if it is set.
if(LIBLZMA_ROOT)
  set(_LIBLZMA_SEARCH_ROOT PATHS ${LIBLZMA_ROOT} NO_DEFAULT_PATH)
  list(APPEND _LIBLZMA_SEARCHES _LIBLZMA_SEARCH_ROOT)
endif()

# Normal search.
list(APPEND _LIBLZMA_SEARCHES _LIBLZMA_SEARCH_NORMAL)

set(LIBLZMA_NAMES liblzma)
set(LIBLZMA_NAMES_DEBUG liblzmad)

# Try each search configuration.
foreach(search ${_LIBLZMA_SEARCHES})
  find_path(LIBLZMA_INCLUDE_DIR NAMES lzma.h ${${search}} PATH_SUFFIXES include)
endforeach()

# Allow LIBLZMA_LIBRARY to be set manually, as the location of the liblzma library
if(NOT LIBLZMA_LIBRARY)
  foreach(search ${_LIBLZMA_SEARCHES})
    find_library(LIBLZMA_LIBRARY_RELEASE NAMES ${LIBLZMA_NAMES} NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
    find_library(LIBLZMA_LIBRARY_DEBUG NAMES ${LIBLZMA_NAMES_DEBUG} NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
  endforeach()

  include(SelectLibraryConfigurations)
  select_library_configurations(LIBLZMA)
endif()

unset(LIBLZMA_NAMES)
unset(LIBLZMA_NAMES_DEBUG)

if(LIBLZMA_INCLUDE_DIR AND EXISTS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h")
    file(STRINGS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h" LIBLZMA_HEADER_CONTENTS REGEX "#define LZMA_VERSION_[A-Z]+ [0-9]+")

    string(REGEX REPLACE ".*#define LZMA_VERSION_MAJOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MAJOR "${LIBLZMA_HEADER_CONTENTS}")
    string(REGEX REPLACE ".*#define LZMA_VERSION_MINOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MINOR "${LIBLZMA_HEADER_CONTENTS}")
    string(REGEX REPLACE ".*#define LZMA_VERSION_PATCH ([0-9]+).*" "\\1" LIBLZMA_VERSION_PATCH "${LIBLZMA_HEADER_CONTENTS}")

    set(LIBLZMA_VERSION_STRING "${LIBLZMA_VERSION_MAJOR}.${LIBLZMA_VERSION_MINOR}.${LIBLZMA_VERSION_PATCH}")
    unset(LIBLZMA_HEADER_CONTENTS)
endif()

# We're using new code known now as XZ, even library still been called LZMA
# it can be found in http://tukaani.org/xz/
# Avoid using old codebase
if (LIBLZMA_LIBRARY)
   include(CheckLibraryExists)
   set(CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
   set(CMAKE_REQUIRED_QUIET ${LibLZMA_FIND_QUIETLY})
   set(CMAKE_REQUIRED_DEFINITIONS LZMA_API_STATIC)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_auto_decoder "" LIBLZMA_HAS_AUTO_DECODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_easy_encoder "" LIBLZMA_HAS_EASY_ENCODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_lzma_preset "" LIBLZMA_HAS_LZMA_PRESET)
   set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_SAVE})
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBLZMA  REQUIRED_VARS  LIBLZMA_INCLUDE_DIR
                                                          LIBLZMA_LIBRARY
                                                          #LIBLZMA_HAS_AUTO_DECODER
                                                          #LIBLZMA_HAS_EASY_ENCODER
                                                          #LIBLZMA_HAS_LZMA_PRESET
                                           VERSION_VAR    LIBLZMA_VERSION_STRING
                                 )

if(LIBLZMA_FOUND)
    set(LIBLZMA_INCLUDE_DIRS ${LIBLZMA_INCLUDE_DIR})

    if(NOT LIBLZMA_LIBRARIES)
      set(LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})
    endif()

    if(NOT TARGET LIBLZMA::LIBLZMA)
      add_library(LIBLZMA::LIBLZMA UNKNOWN IMPORTED)
      set_target_properties(LIBLZMA::LIBLZMA PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIBLZMA_INCLUDE_DIRS}")
      set_target_properties(LIBLZMA::LIBLZMA PROPERTIES INTERFACE_COMPILE_DEFINITIONS LZMA_API_STATIC)

      if(LIBLZMA_LIBRARY_RELEASE)
        set_property(TARGET LIBLZMA::LIBLZMA APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(LIBLZMA::LIBLZMA PROPERTIES IMPORTED_LOCATION_RELEASE "${LIBLZMA_LIBRARY_RELEASE}")
      endif()

      if(LIBLZMA_LIBRARY_DEBUG)
        set_property(TARGET LIBLZMA::LIBLZMA APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(LIBLZMA::LIBLZMA PROPERTIES IMPORTED_LOCATION_DEBUG "${LIBLZMA_LIBRARY_DEBUG}")
      endif()

      if(NOT LIBLZMA_LIBRARY_RELEASE AND NOT LIBLZMA_LIBRARY_DEBUG)
        set_property(TARGET LIBLZMA::LIBLZMA APPEND PROPERTY IMPORTED_LOCATION "${LIBLZMA_LIBRARY}")
      endif()
    endif()
endif()

mark_as_advanced( LIBLZMA_INCLUDE_DIR LIBLZMA_LIBRARY )
