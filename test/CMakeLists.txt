include_directories(${gtest_SOURCE_DIR}/include)
add_executable(tests EXCLUDE_FROM_ALL relic_wrapper_tests.cpp libforwardsec_tests.cpp)
target_link_libraries (tests LINK_PUBLIC forwardsec  gtest_main)
add_dependencies(tests gtest_main forwardsec)
add_test(tests tests)
add_dependencies(check tests) #check command now depends on test