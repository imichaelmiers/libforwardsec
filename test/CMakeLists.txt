include_directories(${gtest_SOURCE_DIR}/include)

add_executable(tests tests.cpp)
target_link_libraries (tests LINK_PUBLIC forwardsec relic gtest_main)
