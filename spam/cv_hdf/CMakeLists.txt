set(cv_hdf_SRC
    src/hdf5.cpp

    # Headers
    src/precomp.hpp
    include/opencv2/hdf.hpp
    include/opencv2/hdf/hdf5.hpp
)
if(MSVC)
  set_property(SOURCE src/hdf5.cpp APPEND PROPERTY COMPILE_OPTIONS "/wd4267" "/utf-8")
endif()

# make lib for cv_hdf_LIB
add_library(cv_hdf STATIC "${cv_hdf_SRC}")
set_target_properties (cv_hdf PROPERTIES FOLDER thirdparty)

set(libs)
list(APPEND libs hdf5::hdf5_cpp-static)
list(APPEND libs opencv_core)
list(APPEND libs opencv_imgcodecs)

if(MSVC)
  target_compile_definitions(cv_hdf PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(cv_hdf PRIVATE "/MP")
endif()

target_include_directories(cv_hdf PRIVATE include)
target_link_libraries(cv_hdf PRIVATE ${libs})