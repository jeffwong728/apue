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
list(APPEND libs GLIB::glib)
list(APPEND libs GLIBMM::glibmm)
target_link_libraries(util PRIVATE ${libs})
