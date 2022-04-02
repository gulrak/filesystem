cmake_minimum_required(VERSION 3.7.2)
project(ghcfilesystem)

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0110)
    cmake_policy(SET CMP0110 NEW)
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(GHC_FILESYSTEM_BUILD_TESTING "Enable tests" ON)
    option(GHC_FILESYSTEM_BUILD_EXAMPLES "Build examples" ON)
    option(GHC_FILESYSTEM_WITH_INSTALL "With install target" ON)
else()
    option(GHC_FILESYSTEM_BUILD_EXAMPLES "Build examples" OFF)
    option(GHC_FILESYSTEM_BUILD_TESTING "Enable tests" OFF)
    option(GHC_FILESYSTEM_WITH_INSTALL "With install target" ON)
endif()
option(GHC_FILESYSTEM_BUILD_STD_TESTING "Enable STD tests" ${GHC_FILESYSTEM_BUILD_TESTING})
if(NOT DEFINED GHC_FILESYSTEM_TEST_COMPILE_FEATURES)
    set(GHC_FILESYSTEM_TEST_COMPILE_FEATURES ${CMAKE_CXX_COMPILE_FEATURES})
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(NOT CYGWIN)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
endif()
if(CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "CMAKE_CXX_STANDARD is less than 11, ghc::filesystem only works with C++11 and above.")
endif()
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILE_FEATURES: ${CMAKE_CXX_COMPILE_FEATURES}")

add_library(ghc_filesystem INTERFACE)
target_include_directories(ghc_filesystem INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_options(ghc_filesystem INTERFACE "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
target_compile_options(ghc_filesystem INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(GHC_FILESYSTEM_BUILD_TESTING OR GHC_FILESYSTEM_BUILD_EXAMPLES)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
    include(GhcHelper)

    if(GHC_FILESYSTEM_BUILD_TESTING)
        enable_testing()
        add_subdirectory(test)
    endif()

    if(GHC_FILESYSTEM_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()

if(GHC_FILESYSTEM_WITH_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS ghc_filesystem EXPORT ghc_filesystem-targets)
    install(EXPORT ghc_filesystem-targets NAMESPACE ghcFilesystem:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ghc_filesystem")
    export(EXPORT ghc_filesystem-targets NAMESPACE ghcFilesystem:: FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/ghc_filesystem-targets.cmake")
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
        "${PROJECT_BINARY_DIR}/cmake/ghc_filesystem-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ghc_filesystem"
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR)
    install(FILES "${PROJECT_BINARY_DIR}/cmake/ghc_filesystem-config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ghc_filesystem")
    add_library(ghcFilesystem::ghc_filesystem ALIAS ghc_filesystem)
endif()
