# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindIconv
---------

This module finds the ``iconv()`` POSIX.1 functions on the system.
These functions might be provided in the regular C library or externally
in the form of an additional library.

The following variables are provided to indicate iconv support:

.. variable:: Iconv_FOUND

  Variable indicating if the iconv support was found.

.. variable:: Iconv_INCLUDE_DIRS

  The directories containing the iconv headers.

.. variable:: Iconv_LIBRARIES

  The iconv libraries to be linked.

.. variable:: Iconv_IS_BUILT_IN

  A variable indicating whether iconv support is stemming from the
  C library or not. Even if the C library provides `iconv()`, the presence of
  an external `libiconv` implementation might lead to this being false.

Additionally, the following :prop_tgt:`IMPORTED` target is being provided:

.. variable:: Iconv::Iconv

  Imported target for using iconv.

The following cache variables may also be set:

.. variable:: Iconv_INCLUDE_DIR

  The directory containing the iconv headers.

.. variable:: Iconv_LIBRARY

  The iconv library (if not implicitly given in the C library).

.. note::
  On POSIX platforms, iconv might be part of the C library and the cache
  variables ``Iconv_INCLUDE_DIR`` and ``Iconv_LIBRARY`` might be empty.

#]=======================================================================]

include(CMakePushCheckState)
if(CMAKE_C_COMPILER_LOADED)
  include(CheckCSourceCompiles)
elseif(CMAKE_CXX_COMPILER_LOADED)
  include(CheckCXXSourceCompiles)
else()
  # If neither C nor CXX are loaded, implicit iconv makes no sense.
  set(Iconv_IS_BUILT_IN FALSE)
endif()

# iconv can only be provided in libc on a POSIX system.
# If any cache variable is already set, we'll skip this test.
if(NOT DEFINED Iconv_IS_BUILT_IN)
  if(UNIX AND NOT DEFINED Iconv_INCLUDE_DIR AND NOT DEFINED Iconv_LIBRARY)
    cmake_push_check_state(RESET)
    # We always suppress the message here: Otherwise on supported systems
    # not having iconv in their C library (e.g. those using libiconv)
    # would always display a confusing "Looking for iconv - not found" message
    set(CMAKE_FIND_QUIETLY TRUE)
    # The following code will not work, but it's sufficient to see if it compiles.
    # Note: libiconv will define the iconv functions as macros, so CheckSymbolExists
    # will not yield correct results.
    set(Iconv_IMPLICIT_TEST_CODE
      "
      #include <stddef.h>
      #include <iconv.h>
      int main() {
        char *a, *b;
        size_t i, j;
        iconv_t ic;
        ic = iconv_open(\"to\", \"from\");
        iconv(ic, &a, &i, &b, &j);
        iconv_close(ic);
      }
      "
    )
    if(CMAKE_C_COMPILER_LOADED)
      check_c_source_compiles("${Iconv_IMPLICIT_TEST_CODE}" Iconv_IS_BUILT_IN)
    else()
      check_cxx_source_compiles("${Iconv_IMPLICIT_TEST_CODE}" Iconv_IS_BUILT_IN)
    endif()
    cmake_pop_check_state()
  else()
    set(Iconv_IS_BUILT_IN FALSE)
  endif()
endif()

if(NOT Iconv_IS_BUILT_IN)
  find_path(Iconv_INCLUDE_DIR
    NAMES "iconv.h"
    DOC "iconv include directory")
  set(Iconv_LIBRARY_NAMES_RELEASE "iconv" "libiconv")
  set(Iconv_LIBRARY_NAMES_DEBUG "iconvd" "libiconvd")
  set(Iconv_charset_LIBRARY_NAMES_RELEASE "libcharset")
  set(Iconv_charset_LIBRARY_NAMES_DEBUG "libcharsetd")
else()
  set(Iconv_INCLUDE_DIR "" CACHE FILEPATH "iconv include directory")
  set(Iconv_LIBRARY_NAMES_RELEASE "c")
  set(Iconv_LIBRARY_NAMES_DEBUG "c")
  set(Iconv_charset_LIBRARY_NAMES_RELEASE "c")
  set(Iconv_charset_LIBRARY_NAMES_DEBUG "c")
endif()

find_library(Iconv_LIBRARY_RELEASE NAMES ${Iconv_LIBRARY_NAMES_RELEASE} DOC "iconv library relaese (potentially the C library)")
find_library(Iconv_LIBRARY_DEBUG NAMES ${Iconv_LIBRARY_NAMES_DEBUG} DOC "iconv library debug (potentially the C library)")
find_library(Iconv_charset_LIBRARY_RELEASE NAMES ${Iconv_charset_LIBRARY_NAMES_RELEASE} DOC "iconv charset library relaese (potentially the C library)")
find_library(Iconv_charset_LIBRARY_DEBUG NAMES ${Iconv_charset_LIBRARY_NAMES_DEBUG} DOC "iconv charset library debug (potentially the C library)")

include(SelectLibraryConfigurations)
select_library_configurations(Iconv)
select_library_configurations(Iconv_charset)

mark_as_advanced(Iconv_INCLUDE_DIR)
mark_as_advanced(Iconv_LIBRARY)
mark_as_advanced(Iconv_charset_LIBRARY)

include(FindPackageHandleStandardArgs)
if(NOT Iconv_IS_BUILT_IN)
  find_package_handle_standard_args(Iconv REQUIRED_VARS Iconv_LIBRARY Iconv_charset_LIBRARY Iconv_INCLUDE_DIR)
else()
  find_package_handle_standard_args(Iconv REQUIRED_VARS Iconv_LIBRARY Iconv_charset_LIBRARY)
endif()

include(SpamCommon)
if(Iconv_FOUND)
  set(Iconv_INCLUDE_DIRS "${Iconv_INCLUDE_DIR}")
  set(Iconv_LIBRARIES "${Iconv_LIBRARY}" "${Iconv_charset_LIBRARY}")

  spam_export_pack(Iconv)
  spam_export_pack_component(Iconv charset)
endif()
