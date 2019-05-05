cmake_minimum_required(VERSION 3.9)
project(ghcfilesystem)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "CMAKE_CXX_STANDARD is less than 11, ghc::filesystem only works with C++11 and above.")
endif()

add_library(ghc_filesystem INTERFACE)
target_sources(ghc_filesystem INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/fs_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/fs_impl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/fs_std.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/fs_std_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ghc/fs_std_impl.hpp
)
target_include_directories(ghc_filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(ghc_filesystem INTERFACE "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
target_compile_options(ghc_filesystem INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
    include(GhcHelper)
    enable_testing()
    add_subdirectory(test)
    add_subdirectory(examples)
endif()
