if(MSVC)
    add_definitions("/utf-8" "/MP" "/wd4244")
endif()

set(stub_SRC
    color.cpp
    
    # -------
    # Headers
    cms-color-types.h
    cms-system.h
    color.h
    colorspace.h
    preferences.h
    strneq.h
    style.h
)

add_library(stub STATIC "${stub_SRC}")
set_target_properties (stub PROPERTIES FOLDER spam)
target_include_directories(stub PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(stub PRIVATE ${CMAKE_BINARY_DIR}/include)

set(libs)
list(APPEND libs VCPKG::glib)
list(APPEND libs VCPKG::glibmm)
target_link_libraries(stub PRIVATE ${libs})
