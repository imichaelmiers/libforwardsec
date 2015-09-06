cmake_minimum_required (VERSION 2.8.11)
project (libforwardsecExample C CXX)
include(CheckCXXCompilerFlag)
if(APPLE) #hack to deal wint OSX 10.10 YOSIMTE 
	include_directories("/usr/local/include", "/usr/include")
endif(APPLE)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


find_library(LIBFORWARDSEC_LIBRARY forwardsec)
IF(NOT LIBFORWARDSEC_LIBRARY)
	message(FATAL_ERROR "libforwardsec not found library not found. Did you build and install it?")
ENDIF(NOT LIBFORWARDSEC_LIBRARY)

#DEPENDENCY CHECKS FOR RELIC 
find_library(RELIC_LIBRARY relic)
IF(NOT RELIC_LIBRARY)
	message(FATAL_ERROR "Relic library not found. Did you  install relic?")
ENDIF(NOT RELIC_LIBRARY)

add_executable(example example.cpp)
target_link_libraries(example ${LIBFORWARDSEC_LIBRARY} ${RELIC_LIBRARY})