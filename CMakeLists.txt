cmake_minimum_required (VERSION 2.6)
project (libforwardsec)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# emulating gnu check command c/o http://www.cmake.org/Wiki/CMakeEmulateMakeCheck
add_custom_target(check COMMAND ${CMAKE_BINARY_DIR}/test/tests PROPERTIES DEPENDS)

enable_testing()
add_subdirectory(src)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_test(NAME test COMMAND tests )

#ADD_TEST(ctest_build_test_code "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target test_code)
#ADD_TEST(ctest_run_test_code test_code)
#SET_TESTS_PROPERTIES ( ctest_run_test_code PROPERTIES DEPENDS ctest_build_test_code)

