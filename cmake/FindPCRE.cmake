# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindPCRE
# ---------
#
# Find the native PCRE headers and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``PCRE::pcre``
#   The PCRE ``pcre`` library, if found.
#
# ``PCRE::pcre16``
#   The PCRE ``pcre16`` library, if found.
#
# ``PCRE::pcre32``
#   The PCRE ``pcre32`` library, if found.
#
# ``PCRE::pcrecpp``
#   The PCRE ``pcrecpp`` library, if found.
#
# ``PCRE::pcreposix``
#   The PCRE ``pcreposix`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``PCRE_INCLUDE_DIRS``
#   where to find pcre.h, etc.
# ``PCRE_LIBRARIES``
#   the libraries to link against to use PCRE.
# ``PCRE_FOUND``
#   true if the pcre headers and libraries were found.
#

# Search PCRE_ROOT first if it is set.

# Look for the header file.
find_path(PCRE_INCLUDE_DIR NAMES pcre.h PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES include NO_DEFAULT_PATH)

include(SelectLibraryConfigurations)

# Look for pcre library.
if(NOT PCRE_pcre_LIBRARY)
  find_library(PCRE_pcre_LIBRARY_RELEASE NAMES pcre NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(PCRE_pcre_LIBRARY_DEBUG NAMES pcred NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
  select_library_configurations(PCRE_pcre)
endif()

# Look for pcre16 library.
if(NOT PCRE_pcre16_LIBRARY)
  find_library(PCRE_pcre16_LIBRARY_RELEASE NAMES pcre16 NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(PCRE_pcre16_LIBRARY_DEBUG NAMES pcre16d NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
  select_library_configurations(PCRE_pcre16)
endif()

# Look for pcre32 library.
if(NOT PCRE_pcre32_LIBRARY)
  find_library(PCRE_pcre32_LIBRARY_RELEASE NAMES pcre32 NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(PCRE_pcre32_LIBRARY_DEBUG NAMES pcre32d NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
  select_library_configurations(PCRE_pcre32)
endif()

# Look for pcrecpp library.
if(NOT PCRE_pcrecpp_LIBRARY)
  find_library(PCRE_pcrecpp_LIBRARY_RELEASE NAMES pcrecpp NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(PCRE_pcrecpp_LIBRARY_DEBUG NAMES pcrecppd NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
  select_library_configurations(PCRE_pcrecpp)
endif()

# Look for pcrecpp library.
if(NOT PCRE_pcreposix_LIBRARY)
  find_library(PCRE_pcreposix_LIBRARY_RELEASE NAMES pcreposix NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(PCRE_pcreposix_LIBRARY_DEBUG NAMES pcreposixd NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
  select_library_configurations(PCRE_pcreposix)
endif()

mark_as_advanced(PCRE_INCLUDE_DIR)

if (PCRE_INCLUDE_DIR AND EXISTS "${PCRE_INCLUDE_DIR}/pcre.h")
    file(STRINGS "${PCRE_INCLUDE_DIR}/pcre.h" pcre_version_str
         REGEX "^#[\t ]*define[\t ]+PCRE_(MAJOR|MINOR)[\t ]+[0-9]+$")

    unset(PCRE_VERSION_STRING)
    foreach(VPART MAJOR MINOR)
        foreach(VLINE ${pcre_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+PCRE_${VPART}[\t ]+([0-9]+)$")
                set(PCRE_VERSION_PART "${CMAKE_MATCH_1}")
                if(PCRE_VERSION_STRING)
                    string(APPEND PCRE_VERSION_STRING ".${PCRE_VERSION_PART}")
                else()
                    set(PCRE_VERSION_STRING "${PCRE_VERSION_PART}")
                endif()
            endif()
        endforeach()
    endforeach()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE
  FOUND_VAR
    PCRE_FOUND
  REQUIRED_VARS
    PCRE_INCLUDE_DIR
    PCRE_pcre_LIBRARY
    PCRE_pcre16_LIBRARY
    PCRE_pcre32_LIBRARY
    PCRE_pcrecpp_LIBRARY
    PCRE_pcreposix_LIBRARY
  VERSION_VAR
    PCRE_VERSION_STRING)

mark_as_advanced( PCRE_ROOT )
mark_as_advanced( PCRE_VERSION_STRING )
mark_as_advanced( PCRE_pcre_LIBRARY )
mark_as_advanced( PCRE_pcre16_LIBRARY )
mark_as_advanced( PCRE_pcre32_LIBRARY )
mark_as_advanced( PCRE_pcrecpp_LIBRARY )
mark_as_advanced( PCRE_pcreposix_LIBRARY )

macro(export_pcre_component comp_name)
  if(NOT TARGET PCRE::${comp_name})
      add_library(PCRE::${comp_name} UNKNOWN IMPORTED)
      set_target_properties(PCRE::${comp_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PCRE_INCLUDE_DIRS}")

      if(PCRE_${comp_name}_LIBRARY_RELEASE)
        set_property(TARGET PCRE::${comp_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(PCRE::${comp_name} PROPERTIES IMPORTED_LOCATION_RELEASE "${PCRE_${comp_name}_LIBRARY_RELEASE}")
      endif()

      if(PCRE_${comp_name}_LIBRARY_DEBUG)
        set_property(TARGET PCRE::${comp_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(PCRE::${comp_name} PROPERTIES IMPORTED_LOCATION_DEBUG "${PCRE_${comp_name}_LIBRARY_DEBUG}")
      endif()

      if(NOT PCRE_${comp_name}_LIBRARY_RELEASE AND NOT PCRE_${comp_name}_LIBRARY_DEBUG)
        set_property(TARGET PCRE::${comp_name} APPEND PROPERTY IMPORTED_LOCATION "${PCRE_${comp_name}_LIBRARY}")
      endif()
  endif()
endmacro(export_pcre_component)

# Copy the results to the output variables and target.
if(PCRE_FOUND AND NOT TARGET PCRE::pcre )
    set(PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR})
    if(NOT PCRE_LIBRARIES)
        set(PCRE_LIBRARIES ${PCRE_pcre_LIBRARY} ${PCRE_pcre16_LIBRARY} ${PCRE_pcre32_LIBRARY} ${PCRE_pcrecpp_LIBRARY} ${PCRE_pcreposix_LIBRARY} )
    endif()

    export_pcre_component(pcre)
    export_pcre_component(pcre16)
    export_pcre_component(pcre32)
    export_pcre_component(pcrecpp)
    export_pcre_component(pcreposix)
endif()
