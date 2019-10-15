set(the_description "Machine Vision Package")

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

ocv_define_module(mvlab opencv_core opencv_imgproc opencv_calib3d WRAP python)
ocv_target_link_libraries(${the_module} TBB::tbb TBB::tbbmalloc Boost::filesystem Boost::serialization)
