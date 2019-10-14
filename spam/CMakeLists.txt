include_directories(${CMAKE_CURRENT_LIST_DIR})
if(MSVC)
    add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
endif()

file(COPY ${CMAKE_CURRENT_LIST_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(cv_hdf)
add_subdirectory(helper)
add_subdirectory(svg)
add_subdirectory(util)
add_subdirectory(livarot)
add_subdirectory(stub)
add_subdirectory(ui)
add_subdirectory(unittest)

add_executable(spam WIN32 init.cpp msvc_recommended_pragmas.h)
set_target_properties (spam PROPERTIES FOLDER spam)
target_link_libraries(spam PRIVATE ui_core helper svg util livarot stub cv_hdf unofficial::glib::gobject EXPAT::EXPAT EXIF::EXIF)

if(MSVC)
  target_compile_definitions(spam PRIVATE _CRTDBG_MAP_ALLOC)
endif()

set_target_properties(spam PROPERTIES DEBUG_POSTFIX "_debug")
set_target_properties(spam PROPERTIES RELWITHDEBINFO_POSTFIX "_relwithdebinfo")
set_target_properties(spam PROPERTIES MINSIZEREL_POSTFIX "_minsizerel")

include(GNUInstallDirs)
install(TARGETS spam DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:spam> DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
endif()

include (InstallRequiredSystemLibraries)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake @ONLY)
install (SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)