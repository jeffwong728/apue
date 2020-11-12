if(MSVC)
    add_definitions("/utf-8" "/MP" "/wd4267" "/wd4267" "/wd4244" "/wd4003" "/wd4101" "/wd4305" "/wd4138")
endif()

set(livarot_SRC
	AlphaLigne.cpp
	AVL.cpp
	BitLigne.cpp
	float-line.cpp
	int-line.cpp
	PathConversion.cpp
	Path.cpp
	PathCutting.cpp
	path-description.cpp
	PathOutline.cpp
	PathSimplify.cpp
	PathStroke.cpp
	Shape.cpp
	ShapeDraw.cpp
	ShapeMisc.cpp
	ShapeRaster.cpp
	ShapeSweep.cpp
	sweep-event.cpp
	sweep-tree.cpp
	sweep-tree-list.cpp


	# -------
	# Headers
	AVL.h
	AlphaLigne.h
	BitLigne.h
	Livarot.h
	LivarotDefs.h
	Path.h
	Shape.h
	float-line.h
	int-line.h
	path-description.h
	sweep-event-queue.h
	sweep-event.h
	sweep-tree-list.h
	sweep-tree.h
)

add_library(livarot STATIC "${livarot_SRC}")
set_target_properties (livarot PROPERTIES FOLDER thirdparty)

if(MSVC)
  target_compile_definitions(livarot PRIVATE _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
endif()
target_include_directories(livarot PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(livarot PRIVATE ${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/spam/stub)

set(libs)
list(APPEND libs VCPKG::glib)
list(APPEND libs VCPKG::glibmm)
list(APPEND libs 2Geom::2geom)
target_link_libraries(livarot PRIVATE ${libs})
