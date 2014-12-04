cmake_minimum_required (VERSION 2.6)
project (libforwardsec)
enable_testing()
add_subdirectory(src)
add_subdirectory(googletest)
add_subdirectory(test)
add_test(NAME test COMMAND tests )

#ADD_TEST(ctest_build_test_code "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target test_code)
#ADD_TEST(ctest_run_test_code test_code)
#SET_TESTS_PROPERTIES ( ctest_run_test_code PROPERTIES DEPENDS ctest_build_test_code)

add_custom_target(check COMMAND ${CMAKE_BINARY_DIR}/test/tests PROPERTIES DEPENDS tests)