cmake_minimum_required(VERSION 3.12.1)
project(bzip2 C)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    get_filename_component(absInstallPath "../../install/${CMAKE_PROJECT_NAME}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(CMAKE_INSTALL_PREFIX "${absInstallPath}" CACHE PATH "Install path prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

add_compile_definitions($<$<CONFIG:Debug>:BZ_DEBUG>)

set(LIBBZ2_SOURCES
    blocksort.c
    huffman.c
    crctable.c
    randtable.c
    compress.c
    decompress.c
    bzlib.c)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELWITHDEBINFO_POSTFIX "_relwithdebinfo")
set(CMAKE_MINSIZEREL_POSTFIX "_minsizerel")

if(MSVC)
    set(pdb_output_dir "${CMAKE_CURRENT_BINARY_DIR}/pdb-files")
    set(CMAKE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
    
    get_cmake_property(is_multi GENERATOR_IS_MULTI_CONFIG)
    if(is_multi)
        set(config_suffix "$<CONFIG>")
    else()
        set(config_suffix "")
    endif()

    include(GNUInstallDirs)
    if(BUILD_SHARED_LIBS)
        set(pdb_dst ${CMAKE_INSTALL_BINDIR})
    else()
        set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
    endif()

    install(DIRECTORY "${pdb_output_dir}/${config_suffix}/" DESTINATION ${pdb_dst})
endif()

add_library(libbz2 ${LIBBZ2_SOURCES})
set_target_properties(libbz2 PROPERTIES ARCHIVE_OUTPUT_NAME bz2)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(libbz2 PRIVATE -DBZ_BUILD_DLL)
endif()

get_target_property(libbz2BaseName libbz2 ARCHIVE_OUTPUT_NAME)
set_target_properties(libbz2 PROPERTIES COMPILE_PDB_NAME_DEBUG "${libbz2BaseName}${CMAKE_DEBUG_POSTFIX}")
set_target_properties(libbz2 PROPERTIES COMPILE_PDB_NAME_RELWITHDEBINFO "${libbz2BaseName}${CMAKE_RELWITHDEBINFO_POSTFIX}")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

install(TARGETS libbz2
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

if(NOT BZIP2_SKIP_TOOLS)
    add_executable(bzip2 bzip2.c ${LIBBZ2_SOURCES})
    add_executable(bzip2recover bzip2recover.c ${LIBBZ2_SOURCES})
    install(TARGETS bzip2 bzip2recover DESTINATION tools)
endif()

if(NOT BZIP2_SKIP_HEADERS)
    install(FILES bzlib.h DESTINATION include)
endif()
