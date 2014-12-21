cmake_minimum_required (VERSION 2.6)
project (libforwardsec C CXX)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Debug. Use cmake -DCMAKE_BUILD_TYPE=Release to make release build ")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

#DEPENDENCY CHECKS FOR RELIC 
find_library(RELIC_LIBRARY relic)
IF(NOT RELIC_LIBRARY)
	message(FATAL_ERROR "Relic library not found. Did you  install relic?")
ENDIF(NOT RELIC_LIBRARY)

find_path(RELIC_HEADER NAMES relic/relic.h)
IF(NOT RELIC_HEADER)
	message(FATAL_ERROR "relic/relic.h headers not found in system. Did you install relic?")
ENDIF(NOT RELIC_HEADER)


#BOOST dependencies for serialization.
find_package(Boost 1.5 COMPONENTS serialization)
if(Boost_SERIALIZATION_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
else()
	message(FATAL_ERROR, "Cannot find boost serialization library")
endif(Boost_SERIALIZATION_FOUND)



# emulating gnu autoconf check command c/o http://www.cmake.org/Wiki/CMakeEmulateMakeCheck
add_custom_target(check COMMAND ${CMAKE_BINARY_DIR}/test/tests PROPERTIES DEPENDS)

enable_testing()
add_subdirectory(src)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_test(NAME test COMMAND tests)