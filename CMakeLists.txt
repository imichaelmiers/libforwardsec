cmake_minimum_required (VERSION 2.6)
project (libforwardsec C)


enable_testing()
add_subdirectory(src)
add_subdirectory(test)
add_test(NAME checkTest COMMAND check)