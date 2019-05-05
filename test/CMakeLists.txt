
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
include(ParseAndAddCatchTests)

add_executable(filesystem_test filesystem_test.cpp catch.hpp)
target_link_libraries(filesystem_test ghc_filesystem)
target_compile_options(filesystem_test PRIVATE
          $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
          $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
          $<$<CXX_COMPILER_ID:MSVC>:/WX>)
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    target_compile_definitions(filesystem_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if(CMAKE_GENERATOR STREQUAL Xcode)
    add_executable(filesystem_test_cov filesystem_test.cpp catch.hpp)
    target_compile_options(filesystem_test_cov PRIVATE "$<$<CONFIG:DEBUG>:--coverage>")
    target_link_libraries(filesystem_test_cov PUBLIC ghc_filesystem PRIVATE --coverage)
endif()
ParseAndAddCatchTests(filesystem_test filesystem_test)
AddExecutableWithStdFS(std_filesystem_test filesystem_test.cpp catch.hpp)

add_executable(multifile_test multi1.cpp multi2.cpp catch.hpp)
target_link_libraries(multifile_test ghc_filesystem)
add_test(multifile_test multifile_test)

add_executable(fwd_impl_test fwd_test.cpp impl_test.cpp)
target_link_libraries(fwd_impl_test ghc_filesystem)
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    target_compile_definitions(fwd_impl_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(fwd_impl_test fwd_impl_test)
