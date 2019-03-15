add_compile_options("/utf-8")
add_executable(test_SpamRgn rgn_unittest.cpp ${PROJECT_SOURCE_DIR}/spam/ui/proc/rgn.cpp)
target_link_libraries(test_SpamRgn PRIVATE TBB::tbb TBB::tbbmalloc Boost::system opencv_core opencv_imgcodecs)

if(MSVC)
  target_compile_definitions(test_SpamRgn PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC NOMINMAX)
  target_compile_options(test_SpamRgn PRIVATE "/MP")
endif()