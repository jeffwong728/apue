# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindFreeGLUT
# ---------
#
# Find the native FreeGLUT headers and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``GLUT::GLUT``
#   The GLUT ``glut`` library, if found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following variables:
#
# ::
#
#   GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#   GLUT_LIBRARIES, the libraries to link against
#   GLUT_FOUND, If false, do not try to use GLUT.
#
# Also defined, but not for general use are:
#
# ::
#
#   GLUT_glut_LIBRARY = the full path to the glut library.

# Try each search configuration.
find_path(GLUT_INCLUDE_DIR NAMES GL/glut.h PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES include NO_DEFAULT_PATH)

# Allow GLUT_glut_LIBRARY to be set manually, as the location of the glut library
if(NOT GLUT_glut_LIBRARY)
  find_library(GLUT_glut_LIBRARY_RELEASE NAMES freeglut NAMES_PER_DIR PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
  find_library(GLUT_glut_LIBRARY_DEBUG NAMES freeglutd NAMES_PER_DIR PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)

  include(SelectLibraryConfigurations)
  select_library_configurations(GLUT_glut)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLUT REQUIRED_VARS GLUT_glut_LIBRARY GLUT_INCLUDE_DIR)

if(GLUT_FOUND)
    set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR})

    if(NOT GLUT_LIBRARIES)
      set(GLUT_LIBRARIES ${GLUT_glut_LIBRARY})
    endif()

    if(NOT TARGET GLUT::GLUT)
      add_library(GLUT::GLUT UNKNOWN IMPORTED)
      set_target_properties(GLUT::GLUT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLUT_INCLUDE_DIRS}")

      if(GLUT_glut_LIBRARY_RELEASE)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(GLUT::GLUT PROPERTIES IMPORTED_LOCATION_RELEASE "${GLUT_glut_LIBRARY_RELEASE}")
      endif()

      if(GLUT_glut_LIBRARY_DEBUG)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(GLUT::GLUT PROPERTIES IMPORTED_LOCATION_DEBUG "${GLUT_glut_LIBRARY_DEBUG}")
      endif()

      if(NOT GLUT_glut_LIBRARY_RELEASE AND NOT GLUT_glut_LIBRARY_DEBUG)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY IMPORTED_LOCATION "${GLUT_glut_LIBRARY}")
      endif()
    endif()
endif()

mark_as_advanced(GLUT_INCLUDE_DIR GLUT_glut_LIBRARY)
