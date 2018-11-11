# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindOpenCL
# ----------
#
# Try to find OpenCL
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``OpenCL::OpenCL``, if
# OpenCL has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   OpenCL_FOUND          - True if OpenCL was found
#   OpenCL_INCLUDE_DIRS   - include directories for OpenCL
#   OpenCL_LIBRARIES      - link against this library to use OpenCL
#   OpenCL_VERSION_STRING - Highest supported OpenCL version (eg. 1.2)
#   OpenCL_VERSION_MAJOR  - The major version of the OpenCL implementation
#   OpenCL_VERSION_MINOR  - The minor version of the OpenCL implementation
#
# The module will also define two cache variables::
#
#   OpenCL_INCLUDE_DIR    - the OpenCL include directory
#   OpenCL_LIBRARY        - the path to the OpenCL library
#

function(_FIND_OPENCL_VERSION)
  include(CheckSymbolExists)
  include(CMakePushCheckState)
  set(CMAKE_REQUIRED_QUIET ${OpenCL_FIND_QUIETLY})

  CMAKE_PUSH_CHECK_STATE()
  foreach(VERSION "2_2" "2_1" "2_0" "1_2" "1_1" "1_0")
    set(CMAKE_REQUIRED_INCLUDES "${OpenCL_INCLUDE_DIR}")

    if(APPLE)
      CHECK_SYMBOL_EXISTS(
        CL_VERSION_${VERSION}
        "${OpenCL_INCLUDE_DIR}/Headers/cl.h"
        OPENCL_VERSION_${VERSION})
    else()
      CHECK_SYMBOL_EXISTS(
        CL_VERSION_${VERSION}
        "${OpenCL_INCLUDE_DIR}/CL/cl.h"
        OPENCL_VERSION_${VERSION})
    endif()

    if(OPENCL_VERSION_${VERSION})
      string(REPLACE "_" "." VERSION "${VERSION}")
      set(OpenCL_VERSION_STRING ${VERSION} PARENT_SCOPE)
      string(REGEX MATCHALL "[0-9]+" version_components "${VERSION}")
      list(GET version_components 0 major_version)
      list(GET version_components 1 minor_version)
      set(OpenCL_VERSION_MAJOR ${major_version} PARENT_SCOPE)
      set(OpenCL_VERSION_MINOR ${minor_version} PARENT_SCOPE)
      break()
    endif()
  endforeach()
  CMAKE_POP_CHECK_STATE()
endfunction()

find_path(OpenCL_INCLUDE_DIR NAMES CL/cl.h OpenCL/cl.h PATH_SUFFIXES include DOC "OpenCL include directory")
_FIND_OPENCL_VERSION()

find_library(OpenCL_LIBRARY_RELEASE NAMES OpenCL DOC "OpenCL library release (potentially the C library)")
find_library(OpenCL_LIBRARY_DEBUG NAMES OpenCLd DOC "OpenCL library debug (potentially the C library)")
find_program(OpenCL_RUNTIME_LIBRARY_RELEASE "OpenCL.dll" PATH_SUFFIXES bin DOC "OpenCL library release (potentially the C library)")
find_program(OpenCL_RUNTIME_LIBRARY_DEBUG "OpenCLd.dll" PATH_SUFFIXES bin DOC "OpenCL library debug (potentially the C library)")

include(SelectLibraryConfigurations)
select_library_configurations(OpenCL)
select_library_configurations(OpenCL_RUNTIME)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpenCL
  FOUND_VAR OpenCL_FOUND
  REQUIRED_VARS OpenCL_LIBRARY OpenCL_INCLUDE_DIR OpenCL_RUNTIME_LIBRARY_DEBUG OpenCL_RUNTIME_LIBRARY_RELEASE
  VERSION_VAR OpenCL_VERSION_STRING)

mark_as_advanced(OpenCL_INCLUDE_DIR OpenCL_LIBRARY OpenCL_RUNTIME)

include(SpamCommon)
if(OpenCL_FOUND AND NOT TARGET OpenCL::OpenCL)
  set(OpenCL_INCLUDE_DIRS "${OpenCL_INCLUDE_DIR}")
  set(OpenCL_LIBRARIES "${OpenCL_LIBRARY}")
  spam_export_shared_pack(OpenCL)
endif()
