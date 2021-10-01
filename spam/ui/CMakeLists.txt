set(ui_core_SRCS
  cmndef.cpp
  evts.cpp
  spam.cpp
  pymain.cpp
  cmds/spamcmd.cpp
  cmds/stationcmd.cpp
  cmds/geomcmd.cpp
  cmds/transcmd.cpp
  cv/cairocanvas.cpp
  cv/cvimagepanel.cpp
  cv/cvtilepanel.cpp
  cv/wxdccanvas.cpp
  dlg/openprojdlg.cpp
  misc/percentvalidator.cpp
  misc/scopedtimer.cpp
  misc/instructiontip.cpp
  misc/histwidget.cpp
  misc/thumbnailctrl.cpp
  misc/spamutility.cpp
  misc/switchbtn.cpp
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
  projs/fixednode.cpp
  projs/profilenode.cpp
  projs/stylecellrenderer.cpp
  toplevel/preferencesdlg.cpp
  toplevel/logpanel.cpp
  toplevel/consolepanel.cpp
  toplevel/pyeditor.cpp
  toplevel/thumbnailpanel.cpp
  toplevel/mainstatus.cpp
  toplevel/maintoolpane.cpp
  toplevel/projpanel.cpp
  toplevel/rootframe.cpp
  toplevel/glpanel.cpp
  toplevel/imgui_impl_gtk3.cpp
  toplevel/imgui_impl_opengl3.cpp
  toolbox/stylebox.cpp
  toolbox/toolbox.cpp
  toolbox/probebox.cpp
  toolbox/matchbox.cpp
  toolbox/geombox.cpp
  toolbox/procbox.cpp
  toolbox/imgflowbox.cpp
  fsm/boxtool.cpp
  fsm/edittool.cpp
  fsm/booltool.cpp
  fsm/notool.cpp
  fsm/probetool.cpp
  fsm/profiletool.cpp
  fsm/proctool.cpp
  fsm/pyramidtool.cpp
  fsm/recttool.cpp
  fsm/linetool.cpp
  fsm/ellipsetool.cpp
  fsm/spamer.cpp
  fsm/transformtool.cpp
  fsm/nodeedittool.cpp
  fsm/polygontool.cpp
  fsm/beziergontool.cpp
  imgproc/basic.cpp
  scripting/pyrect.cpp
  scripting/pystation.cpp
  scripting/pyproject.cpp
  scripting/mvlabcaster.cpp
  scripting/pyentity.cpp
  scripting/pydrawable.cpp
  scripting/pyellipse.cpp
  scripting/pyfixed.cpp
  procflow/flowchart.cpp
  procflow/stepbase.cpp
  procflow/initstep.cpp
  procflow/endstep.cpp
  procflow/cvtstep.cpp
  procflow/threshstep.cpp
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  graphics/glshader.cpp
  graphics/glprog.cpp
  graphics/glcamera.cpp
  graphics/gltexture.cpp
  graphics/dispnode.cpp
  graphics/disp0dmesh.cpp
  graphics/disp1dmesh.cpp
  graphics/disp2dmesh.cpp
  graphics/disp3dmesh.cpp
  graphics/disptree.cpp
  graphics/glmodeltree.cpp
  graphics/gtkglareawidget.cpp
  graphics/LinearCellDemo.cxx
  graphics/glsl/background_frag.glsl
  graphics/glsl/background_vert.glsl
  graphics/glsl/common_frag.glsl
  graphics/glsl/common_vert.glsl
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
  misc/thumbnailctrl.h
  misc/spamutility.h
  misc/switchbtn.h
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
  projs/fixednode.h
  projs/profilenode.h
  projs/stylecellrenderer.h
  toplevel/preferencesdlg.h
  toplevel/logpanel.h
  toplevel/consolepanel.h
  toplevel/pyeditor.h
  toplevel/thumbnailpanel.h
  toplevel/mainstatus.h
  toplevel/maintoolpane.h
  toplevel/projpanel.h
  toplevel/rootframe.h
  toplevel/glpanel.h
  toplevel/imgui_impl_gtk3.h
  toplevel/imgui_impl_opengl3.h
  toolbox/stylebox.h
  toolbox/toolbox.h
  toolbox/probebox.h
  toolbox/matchbox.h
  toolbox/geombox.h
  toolbox/procbox.h
  toolbox/imgflowbox.h
  fsm/boxtool.h
  fsm/edittool.h
  fsm/booltool.h
  fsm/notool.h
  fsm/probetool.h
  fsm/profiletool.h
  fsm/proctool.h
  fsm/pyramidtool.h
  fsm/recttool.h
  fsm/linetool.h
  fsm/ellipsetool.h
  fsm/spamer.h
  fsm/transformtool.h
  fsm/nodeedittool.h
  fsm/beziergontool.h
  fsm/polygontool.h
  fsm/events.h
  imgproc/basic.h
  scripting/pyrect.h
  scripting/pystation.h
  scripting/pyproject.h
  scripting/mvlabcaster.h
  scripting/pyentity.h
  scripting/pydrawable.h
  scripting/pyellipse.h
  scripting/pyfixed.h
  procflow/flowchart.h
  procflow/stepbase.h
  procflow/initstep.h
  procflow/endstep.h
  procflow/stepfwd.h
  procflow/cvtstep.h
  procflow/threshstep.h
  imgui/imconfig.h
  imgui/imgui.h
  imgui/imgui_internal.h
  graphics/glfwd.h
  graphics/glshader.h
  graphics/glprog.h
  graphics/glcamera.h
  graphics/gltexture.h
  graphics/dispnode.h
  graphics/disp0dmesh.h
  graphics/disp1dmesh.h
  graphics/disp2dmesh.h
  graphics/disp3dmesh.h
  graphics/disptree.h
  graphics/glmodeltree.h
  graphics/gtkglareawidget.h
)

if(MSVC)
  set_property(SOURCE toplevel/rootframe.cpp APPEND PROPERTY COMPILE_OPTIONS "/utf-8")
endif()

add_library(ui_core STATIC ${ui_core_SRCS} ${ui_core_HEADERS})
set_target_properties (ui_core PROPERTIES FOLDER spam)

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
IF (WIN32 AND MSVC)
  list(APPEND libs unofficial::cairo::cairo)
  list(APPEND libs VCPKG::gtk3)
  list(APPEND libs VCPKG::gdk3)
  list(APPEND libs VCPKG::cairomm)
  list(APPEND libs VCPKG::gio)
  list(APPEND libs VCPKG::pangocairo)
  list(APPEND libs VCPKG::gdk_pixbuf)
  list(APPEND libs VCPKG::pango)
  list(APPEND libs VCPKG::gobject)
  list(APPEND libs VCPKG::gthread)
  list(APPEND libs VCPKG::glib)
  list(APPEND libs VCPKG::gmodule)
  list(APPEND libs VCPKG::exif)
  list(APPEND libs VCPKG::epoxy)
  list(APPEND libs TBB::tbb TBB::tbbmalloc)
  list(APPEND libs hdf5::hdf5_cpp-shared)
  list(APPEND libs hdf5::hdf5_hl_cpp-shared)
ELSE()
  list(APPEND libs PkgConfig::CAIRO)
  list(APPEND libs PkgConfig::GTK3)
  list(APPEND libs PkgConfig::GLIB)
  list(APPEND libs PkgConfig::GSL)
  list(APPEND libs PkgConfig::CAIROMM)
  list(APPEND libs PkgConfig::GLIBMM)
  list(APPEND libs PkgConfig::SIGCPP)
  list(APPEND libs PkgConfig::EPOXY)
ENDIF()

list(APPEND libs opencv_core)
list(APPEND libs opencv_imgcodecs)
list(APPEND libs opencv_mvlab)
list(APPEND libs Boost::filesystem)
list(APPEND libs Boost::thread)
list(APPEND libs 2Geom::2geom)
list(APPEND libs OpenGL::GLU)

target_link_libraries(ui_core PUBLIC ${libs})
target_link_libraries(ui_core PRIVATE pybind11::embed glm::glm)
target_link_libraries(ui_core PUBLIC ${Python3_LIBRARY_RELEASE})
target_include_directories(ui_core PUBLIC ${Python3_INCLUDE_DIRS})
target_include_directories(ui_core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)
target_link_options(ui_core PUBLIC ${Python3_LINK_OPTIONS})

target_link_libraries(ui_core PRIVATE ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS ui_core MODULES ${VTK_LIBRARIES})

target_include_directories(ui_core PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(ui_core PRIVATE ${OpenCASCADE_LIBRARIES})

if(MSVC) 
  target_compile_definitions(ui_core PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS NOMINMAX VCL_NAMESPACE=vcl)
  target_compile_definitions(ui_core PRIVATE BOOST_PYTHON_NO_LIB BOOST_PYTHON_STATIC_MODULE PY_MAJOR_VERSION=3 PY_MINOR_VERSION=8) #BOOST_DEBUG_PYTHON
  target_compile_definitions(ui_core PRIVATE BOOST_UUID_FORCE_AUTO_LINK)
endif()

target_compile_options(ui_core PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
target_compile_options(ui_core PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>)
target_compile_definitions(ui_core PRIVATE BOOST_BIND_GLOBAL_PLACEHOLDERS IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

target_include_directories(ui_core PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(ui_core PRIVATE ${CMAKE_BINARY_DIR}/include)

IF (WIN32 AND MSVC)
  target_link_libraries(ui_core PUBLIC wxGTK3::mono)
  set_target_properties(ui_core PROPERTIES COMPILE_FLAGS "/wd4828")
ELSE()
  include(${wxWidgets_USE_FILE})
  target_link_libraries(ui_core PUBLIC ${wxWidgets_LIBRARIES})
  target_include_directories(ui_core PUBLIC SYSTEM "${HDF5_INCLUDE_DIRS}")
  target_link_libraries(ui_core PUBLIC "${HDF5_LIBRARIES}" "${HDF5_HL_LIBRARIES}")
  target_compile_definitions(ui_core PUBLIC "${HDF5_DEFINITIONS}")
ENDIF()