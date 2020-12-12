if(MSVC)
    add_definitions("/utf-8" "/MP" "/wd4267")
endif()

set(util_SRC
    units.cpp
    
    # -------
    # Headers
    units.h
    unordered-containers.h
)

add_library(util STATIC "${util_SRC}")
set_target_properties (util PROPERTIES FOLDER thirdparty)
target_include_directories(util PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(util PRIVATE ${CMAKE_BINARY_DIR}/include)

set(libs)
IF (WIN32 AND MSVC)
  list(APPEND libs unofficial::glib::glib)
  list(APPEND libs VCPKG::glibmm)
ELSE()
  list(APPEND libs PkgConfig::GLIB)
  list(APPEND libs PkgConfig::GLIBMM)
ENDIF()
list(APPEND libs 2Geom::2geom)
target_link_libraries(util PRIVATE ${libs})
