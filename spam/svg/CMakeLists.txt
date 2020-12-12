if(MSVC)
    add_definitions("/utf-8" "/MP" "/wd4267" "/wd4267" "/wd4244" "/wd4003" "/wd4101")
endif()

set(svg_SRC
	css-ostringstream.cpp
	path-string.cpp
        #sp-svg.def
	stringstream.cpp
	strip-trailing-zeros.cpp
	svg-affine.cpp
	svg-color.cpp
	svg-angle.cpp
	svg-length.cpp
	svg-path.cpp
	# test-stubs.cpp


	# -------
	# Headers
	css-ostringstream-test.h
	css-ostringstream.h
	path-string.h
	stringstream-test.h
	stringstream.h
	strip-trailing-zeros.h
	svg-affine-test.h
	svg-color-test.h
	svg-color.h
	svg-icc-color.h
	svg-angle.h
	svg-length-test.h
	svg-length.h
	svg-path-geom-test.h
	svg.h
	# test-stubs.h

)

add_library(svg STATIC "${svg_SRC}")
set_target_properties (svg PROPERTIES FOLDER thirdparty)

if(MSVC)
  target_compile_definitions(svg PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(svg PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(svg PRIVATE ${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/stub)

set(libs)
IF (WIN32 AND MSVC)
  list(APPEND libs unofficial::glib::glib)
  list(APPEND libs VCPKG::glibmm)
ELSE()
  list(APPEND libs PkgConfig::GLIB)
  list(APPEND libs PkgConfig::GLIBMM)
ENDIF()
list(APPEND libs 2Geom::2geom)
target_link_libraries(svg PRIVATE ${libs})
