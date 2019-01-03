set(ui_core_SRCS 
  cmndef.cpp
  evts.cpp
  spam.cpp
  cmds/spamcmd.cpp
  cmds/stationcmd.cpp
  cmds/geomcmd.cpp
  cmds/transcmd.cpp
  cv/cairocanvas.cpp
  cv/cvimagepanel.cpp
  cv/cvtilepanel.cpp
  #cv/cvwidget.cpp
  cv/wxdccanvas.cpp
  dlg/openprojdlg.cpp
  misc/percentvalidator.cpp
  misc/scopedtimer.cpp
  misc/instructiontip.cpp
  misc/histwidget.cpp
  projs/drawstyle.cpp
  projs/modelnode.cpp
  projs/nodefactory.cpp
  projs/projnode.cpp
  projs/projtreemodel.cpp
  projs/stationnode.cpp
  projs/ellipsenode.cpp
  projs/rectnode.cpp
  projs/linenode.cpp
  projs/geomnode.cpp
  projs/drawablenode.cpp
  projs/polygonnode.cpp
  projs/beziergonnode.cpp
  projs/stylecellrenderer.cpp
  toplevel/logpanel.cpp
  toplevel/mainstatus.cpp
  toplevel/maintoolpane.cpp
  toplevel/projpanel.cpp
  toplevel/rootframe.cpp
  toolbox/stylebox.cpp
  toolbox/toolbox.cpp
  toolbox/probebox.cpp
  toolbox/matchbox.cpp
  toolbox/geombox.cpp
  toolbox/procbox.cpp
  fsm/boxtool.cpp
  fsm/edittool.cpp
  fsm/booltool.cpp
  fsm/notool.cpp
  fsm/probetool.cpp
  fsm/recttool.cpp
  fsm/linetool.cpp
  fsm/ellipsetool.cpp
  fsm/spamer.cpp
  fsm/transformtool.cpp
  fsm/nodeedittool.cpp
  fsm/polygontool.cpp
  fsm/beziergontool.cpp
  proc/rgn.cpp
)

set(ui_core_HEADERS 
  cmndef.h
  evts.h
  spam.h
  cmds/cmdsfwd.h
  cmds/spamcmd.h
  cmds/stationcmd.h
  cmds/geomcmd.h
  cmds/transcmd.h
  cv/cairocanvas.h
  cv/cvimagepanel.h
  cv/cvtilepanel.h
  #cv/cvwidget.h
  cv/wxdccanvas.h
  dlg/openprojdlg.h
  misc/percentvalidator.h
  misc/scopedtimer.h
  misc/instructiontip.h
  misc/histwidget.h
  projs/drawstyle.h
  projs/modelfwd.h
  projs/modelnode.h
  projs/nodefactory.h
  projs/projnode.h
  projs/projtreemodel.h
  projs/stationnode.h
  projs/drawablenode.h
  projs/polygonnode.h
  projs/rectnode.h
  projs/linenode.h
  projs/ellipsenode.h
  projs/beziergonnode.h
  projs/geomnode.h
  projs/stylecellrenderer.h
  toplevel/logpanel.h
  toplevel/mainstatus.h
  toplevel/maintoolpane.h
  toplevel/projpanel.h
  toplevel/rootframe.h
  toolbox/stylebox.h
  toolbox/toolbox.h
  toolbox/probebox.h
  toolbox/matchbox.h
  toolbox/geombox.h
  toolbox/procbox.h
  fsm/boxtool.h
  fsm/edittool.h
  fsm/booltool.h
  fsm/notool.h
  fsm/probetool.h
  fsm/recttool.h
  fsm/linetool.h
  fsm/ellipsetool.h
  fsm/spamer.h
  fsm/transformtool.h
  fsm/nodeedittool.h
  fsm/beziergontool.h
  fsm/polygontool.h
  fsm/events.h
  proc/rgn.h
)

if(MSVC)
  set_property(SOURCE toplevel/rootframe.cpp APPEND PROPERTY COMPILE_OPTIONS "/utf-8")
endif()

add_library(ui_core STATIC ${ui_core_SRCS} ${ui_core_HEADERS})

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group(${ui_core_SRCS})
assign_source_group(${ui_core_HEADERS})

set(libs)
list(APPEND libs CAIRO::cairo)
list(APPEND libs CAIROMM::CAIROMM)
list(APPEND libs SKIA::skia)
list(APPEND libs SKIA::pathkit)
list(APPEND libs TBB::tbb TBB::tbbmalloc)
list(APPEND libs hdf5::hdf5_cpp-static)
list(APPEND libs hdf5::hdf5_hl_cpp-static)
list(APPEND libs opencv_core)
list(APPEND libs opencv_imgcodecs)
list(APPEND libs Python2::Python)
list(APPEND libs Boost::python27)
list(APPEND libs Boost::filesystem)
list(APPEND libs Boost::thread)
list(APPEND libs Boost::locale)
list(APPEND libs Freetype::Freetype)
list(APPEND libs Pixman::Pixman)
list(APPEND libs PNG::PNG)
list(APPEND libs JPEG::jpeg)
list(APPEND libs JPEG::turbo)
list(APPEND libs TIFF::TIFF)
list(APPEND libs FontConfig::FontConfig)
list(APPEND libs BZip2::BZip2)
list(APPEND libs EXPAT::EXPAT)
list(APPEND libs LIBLZMA::LIBLZMA)
list(APPEND libs Iconv::Iconv)
list(APPEND libs Iconv::charset)
list(APPEND libs "sigc-3.0")
list(APPEND libs GLIB::glib)
list(APPEND libs GLIB::gobject)
list(APPEND libs GSL::gsl)
list(APPEND libs GSL::gslcblas)
list(APPEND libs WXSVG::WXSVG)

target_link_libraries(ui_core PUBLIC ${libs})

if(MSVC)
  target_compile_definitions(ui_core PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC)
  target_compile_definitions(ui_core PRIVATE BOOST_PYTHON_NO_LIB BOOST_PYTHON_STATIC_LIB BOOST_PYTHON_STATIC_MODULE PY_MAJOR_VERSION=2 PY_MINOR_VERSION=7)
  target_compile_options(ui_core PRIVATE "/MP")
endif()

target_include_directories(ui_core PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(ui_core PRIVATE ${CMAKE_BINARY_DIR}/include)
target_include_directories(ui_core PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_compile_definitions(ui_core PRIVATE ${wxWidgets_DEFINITIONS})
target_compile_definitions(ui_core PRIVATE $<$<CONFIG:Debug>:${wxWidgets_DEFINITIONS_DEBUG}>)
target_compile_options(ui_core PRIVATE ${wxWidgets_CXX_FLAGS})
target_link_libraries(ui_core PUBLIC ${wxWidgets_LIBRARIES})