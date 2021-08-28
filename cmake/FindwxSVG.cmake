set(wxSVG_DIR "$ENV{SPAM_ROOT_DIR}/jane/install/wxSVG")
set(wxSVG_INCLUDE_DIR "${wxSVG_DIR}/include")

set(name_static_libs)
set(d_static_libs)
set(r_static_libs)
set(rd_static_libs)

list(APPEND name_static_libs wxSVG)
list(APPEND d_static_libs wxsvg_d)
list(APPEND r_static_libs wxsvg)
list(APPEND rd_static_libs wxsvg_debinfo)

include(SpamCommon)
include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

foreach(name_static_lib d_static_lib r_static_lib rd_static_lib IN ZIP_LISTS name_static_libs d_static_libs r_static_libs rd_static_libs)
    find_library(wxSVG_${name_static_lib}_LIBRARY_DEBUG NAMES ${d_static_lib} PATHS "${wxSVG_DIR}/lib" NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH)
    find_library(wxSVG_${name_static_lib}_LIBRARY_RELEASE NAMES ${r_static_lib} PATHS "${wxSVG_DIR}/lib" NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH)
    find_library(wxSVG_${name_static_lib}_LIBRARY_RELWITHDEBINFO NAMES ${rd_static_lib} PATHS "${wxSVG_DIR}/lib" NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH)

    select_library_configurations(wxSVG_${name_static_lib})
    find_package_handle_standard_args(wxSVG::${name_static_lib} REQUIRED_VARS wxSVG_${name_static_lib}_LIBRARY HANDLE_COMPONENTS NAME_MISMATCHED)

    if(wxSVG::${name_static_lib}_FOUND)
      set(wxSVG_INCLUDE_DIRS "${wxSVG_INCLUDE_DIR}")
      spam_export_pack_component(wxSVG ${name_static_lib})
    endif()
endforeach()
