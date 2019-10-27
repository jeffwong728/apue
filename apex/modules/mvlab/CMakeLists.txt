set(the_description "Machine Vision Package")
add_compile_options("/utf-8")

cmake_policy(PUSH)
cmake_policy(SET CMP0074 NEW)

IF(NOT TARGET TBB::tbb)
  find_package(TBB CONFIG REQUIRED tbb)
ENDIF()

IF(NOT TARGET TBB::tbbmalloc)
  find_package(TBB CONFIG REQUIRED tbbmalloc)
ENDIF()

IF(NOT TARGET Boost::filesystem)
  find_package(Boost REQUIRED COMPONENTS filesystem)
ENDIF()

IF(NOT TARGET Boost::serialization)
  find_package(Boost REQUIRED COMPONENTS serialization)
ENDIF()

list(APPEND CMAKE_MODULE_PATH "$ENV{SPAM_ROOT_DIR}cmake")
set(Lib2Geom_ROOT "$ENV{SPAM_ROOT_DIR}install/lib2geom")
find_package(Lib2Geom REQUIRED)
find_package(CAIROMM REQUIRED)
find_package(unofficial-cairo CONFIG REQUIRED)
find_package(unofficial-glib CONFIG REQUIRED)
find_package(GLIBMM REQUIRED)
find_package(libsigcpp CONFIG REQUIRED)
find_package(GSL REQUIRED)
find_package(double-conversion CONFIG REQUIRED)
find_package(vectorclass CONFIG REQUIRED)
list(POP_BACK CMAKE_MODULE_PATH)

cmake_policy(POP)

ocv_define_module(mvlab opencv_core opencv_imgproc opencv_calib3d WRAP python)
ocv_target_link_libraries(${the_module} LINK_PRIVATE TBB::tbb TBB::tbbmalloc Boost::filesystem Boost::serialization)
ocv_target_link_libraries(${the_module} LINK_PRIVATE unofficial::cairo::cairo unofficial::glib::glib GLIBMM::glibmm CAIROMM::CAIROMM)
ocv_target_link_libraries(${the_module} LINK_PRIVATE LIB2GEOM::2geom sigc-3.0 GSL::gsl double-conversion::double-conversion vectorclass::vectorclass)
target_compile_definitions(${the_module} PRIVATE NOMINMAX __AVX2__ VCL_NAMESPACE=vcl)
target_compile_options(${the_module} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>)
