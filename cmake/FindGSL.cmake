# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindGSL
# --------
#
# Find the native GSL includes and libraries.
#
# The GNU Scientific Library (GSL) is a numerical library for C and C++
# programmers. It is free software under the GNU General Public
# License.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# If GSL is found, this module defines the following :prop_tgt:`IMPORTED`
# targets::
#
#  GSL::gsl      - The main GSL library.
#  GSL::gslcblas - The CBLAS support library used by GSL.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project::
#
#  GSL_FOUND          - True if GSL found on the local system
#  GSL_INCLUDE_DIRS   - Location of GSL header files.
#  GSL_LIBRARIES      - The GSL libraries.
#  GSL_VERSION        - The version of the discovered GSL install.
#
# Hints
# ^^^^^
#
# Set ``GSL_ROOT_DIR`` to a directory that contains a GSL installation.
#
# This script expects to find libraries at ``$GSL_ROOT_DIR/lib`` and the GSL
# headers at ``$GSL_ROOT_DIR/include/gsl``.  The library directory may
# optionally provide Release and Debug folders. If available, the libraries
# named ``gsld``, ``gslblasd`` or ``cblasd`` are recognized as debug libraries.
# For Unix-like systems, this script will use ``$GSL_ROOT_DIR/bin/gsl-config``
# (if found) to aid in the discovery of GSL.
#
# Cache Variables
# ^^^^^^^^^^^^^^^
#
# This module may set the following variables depending on platform and type
# of GSL installation discovered.  These variables may optionally be set to
# help this module find the correct files::
#
#  GSL_CBLAS_LIBRARY       - Location of the GSL CBLAS library.
#  GSL_CBLAS_LIBRARY_DEBUG - Location of the debug GSL CBLAS library (if any).
#  GSL_CONFIG_EXECUTABLE   - Location of the ``gsl-config`` script (if any).
#  GSL_LIBRARY             - Location of the GSL library.
#  GSL_LIBRARY_DEBUG       - Location of the debug GSL library (if any).
#
include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

#=============================================================================
# If the user has provided ``GSL_ROOT_DIR``, use it!  Choose items found
# at this location over system locations.
if( EXISTS "$ENV{GSL_ROOT_DIR}" )
  file( TO_CMAKE_PATH "$ENV{GSL_ROOT_DIR}" GSL_ROOT_DIR )
  set( GSL_ROOT_DIR "${GSL_ROOT_DIR}" CACHE PATH "Prefix for GSL installation." )
endif()
if( NOT EXISTS "${GSL_ROOT_DIR}" )
  set( GSL_USE_PKGCONFIG ON )
endif()

#=============================================================================
# As a first try, use the PkgConfig module.  This will work on many
# *NIX systems.  See :module:`findpkgconfig`
# This will return ``GSL_INCLUDEDIR`` and ``GSL_LIBDIR`` used below.
if( GSL_USE_PKGCONFIG )
  find_package(PkgConfig)
  pkg_check_modules( GSL QUIET gsl )

  if( EXISTS "${GSL_INCLUDEDIR}" )
    get_filename_component( GSL_ROOT_DIR "${GSL_INCLUDEDIR}" DIRECTORY CACHE)
  endif()
endif()

#=============================================================================
# Set GSL_INCLUDE_DIRS and GSL_LIBRARIES. If we skipped the PkgConfig step, try
# to find the libraries at $GSL_ROOT_DIR (if provided) or in standard system
# locations.  These find_library and find_path calls will prefer custom
# locations over standard locations (HINTS).  If the requested file is not found
# at the HINTS location, standard system locations will be still be searched
# (/usr/lib64 (Redhat), lib/i386-linux-gnu (Debian)).

find_path( GSL_INCLUDE_DIR NAMES gsl/gsl_sf.h PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES include NO_DEFAULT_PATH)

find_library( GSL_gsl_LIBRARY_RELEASE NAMES gsl PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library( GSL_gslcblas_LIBRARY_RELEASE NAMES gslcblas cblas PATHS $ENV{VCPKG_DIR} PATH_SUFFIXES lib NO_DEFAULT_PATH)

find_library( GSL_gsl_LIBRARY_DEBUG NAMES gsld gsl PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library( GSL_gslcblas_LIBRARY_DEBUG NAMES gslcblasd cblasd gslcblas cblas PATHS $ENV{VCPKG_DIR}/debug PATH_SUFFIXES lib NO_DEFAULT_PATH)

select_library_configurations(GSL_gsl)
select_library_configurations(GSL_gslcblas)

# If we didn't use PkgConfig, try to find the version via gsl-config or by
# reading gsl_version.h.
if( NOT GSL_VERSION )
  # 1. If gsl-config exists, query for the version.
  find_program( GSL_CONFIG_EXECUTABLE
    NAMES gsl-config
    HINTS "${GSL_ROOT_DIR}/bin"
    )
  if( EXISTS "${GSL_CONFIG_EXECUTABLE}" )
    execute_process(
      COMMAND "${GSL_CONFIG_EXECUTABLE}" --version
      OUTPUT_VARIABLE GSL_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE )
  endif()

  # 2. If gsl-config is not available, try looking in gsl/gsl_version.h
  if( NOT GSL_VERSION AND EXISTS "${GSL_INCLUDE_DIRS}/gsl/gsl_version.h" )
    file( STRINGS "${GSL_INCLUDE_DIRS}/gsl/gsl_version.h" gsl_version_h_contents REGEX "define GSL_VERSION" )
    string( REGEX REPLACE ".*([0-9]\\.[0-9][0-9]?).*" "\\1" GSL_VERSION ${gsl_version_h_contents} )
  endif()

  # might also try scraping the directory name for a regex match "gsl-X.X"
endif()

#=============================================================================
# handle the QUIETLY and REQUIRED arguments and set GSL_FOUND to TRUE if all
# listed variables are TRUE
find_package_handle_standard_args( GSL
  FOUND_VAR
    GSL_FOUND
  REQUIRED_VARS
    GSL_INCLUDE_DIR
    GSL_gsl_LIBRARY
    GSL_gslcblas_LIBRARY
  VERSION_VAR
    GSL_VERSION
    )

mark_as_advanced( GSL_ROOT_DIR GSL_VERSION GSL_gsl_LIBRARY GSL_INCLUDE_DIR
  GSL_gslcblas_LIBRARY GSL_gsl_LIBRARY_DEBUG GSL_gslcblas_LIBRARY_DEBUG
  GSL_USE_PKGCONFIG GSL_CONFIG )

if( GSL_FOUND AND NOT TARGET GSL::gsl )
  set( GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR} )
  set( GSL_LIBRARIES ${GSL_gsl_LIBRARY} ${GSL_gslcblas_LIBRARY} )
  include(SpamCommon)
  spam_export_pack_component(GSL gsl)
  spam_export_pack_component(GSL gslcblas)
  set(GSL_LIBRARY ${GSL_gsl_LIBRARY})
  set(GSL_LIBRARY_DEBUG ${GSL_gsl_LIBRARY_DEBUG})
  set(GSL_LIBRARY_RELEASE ${GSL_gsl_LIBRARY_RELEASE})
endif()
