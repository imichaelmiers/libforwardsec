cmake_minimum_required (VERSION 2.8.11)
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

CHECK_CXX_COMPILER_FLAG("-Wall"  COMPILER_SUPPORTS_WALL)
CHECK_CXX_COMPILER_FLAG("-Wextra"  COMPILER_SUPPORTS_WEXTRA)
if(COMPILER_SUPPORTS_WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif(COMPILER_SUPPORTS_WALL)
if(COMPILER_SUPPORTS_WEXTRA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif(COMPILER_SUPPORTS_WEXTRA)
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

#DEPENDENCY CHECK FOR cereal (USERED FOR SERIALIZATION)
find_path(CEREAL_HEADER NAMES cereal/types/vector.hpp)
IF(NOT CEREAL_HEADER)
	message(FATAL_ERROR "cereal/types/vector.hpp headers not found in system. Did you install cereal(https://github.com/USCiLab/cereal)?")
ENDIF(NOT CEREAL_HEADER)


# Configuration options 


option(RELICXX_UNCONST "Cast const values to non-const when passing to relic functions which are const but not marked as such. (e.g read_bin,mul,etc)." ON)
option(OPEN_MP "Use OpenMP for parallelizing decrypt." OFF)
option(RELICXX_MOVE "Use C++11 move semantics for  ZR,G1,G2" ON)
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(RELICXX_MOVEZR "Use C++11 move semantics for ZR" OFF "NOT RELICXX_MOVE" ON)
CMAKE_DEPENDENT_OPTION(RELICXX_MOVEG1 "Use C++11 move semantics for G1" OFF "NOT RELICXX_MOVE" ON)
CMAKE_DEPENDENT_OPTION(RELICXX_MOVEG2 "Use C++11 move semantics for G2" OFF "NOT RELICXX_MOVE" ON)
CMAKE_DEPENDENT_OPTION(RELICXX_MOVEGT "Use C++11 move semantics for GT" OFF "NOT RELICXX_MOVE" ON)



if(USE_OPENMP)
	find_package(OpenMP REQUIRED)
	if (OPENMP_FOUND)
	    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	    add_definitions(-DRELICXX_USE_OPENMP)
	 else()
	 	message(WARNING "Cannot find OpenMP. OpenMP now dissalbled and  USE_OPENMP = Off")
	endif(OPENMP_FOUND)
endif(USE_OPENMP)

if(RELICXX_UNCONST)
	add_definitions(-DRELICXX_UNCONST)
endif(RELICXX_UNCONST)

if(RELICXX_MOVE)
	add_definitions(-DRELICXX_MOVE)
endif(RELICXX_MOVE)
if(RELICXX_MOVEZR)
	add_definitions(-DRELICXX_MOVEZR)
endif(RELICXX_MOVEZR)
if(RELICXX_MOVEG1)
	add_definitions(-DRELICXX_MOVEG1)
endif(RELICXX_MOVEG1)
if(RELICXX_MOVEG2)
	add_definitions(-DRELICXX_MOVEG2)
endif(RELICXX_MOVEG2)
if(RELICXX_MOVEGT)
	add_definitions(-DRELICXX_MOVEGT)
endif(RELICXX_MOVEGT)

INCLUDE_DIRECTORIES(include/forwardsec)



# emulating gnu autoconf check command c/o http://www.cmake.org/Wiki/CMakeEmulateMakeCheck
add_custom_target(check COMMAND ${CMAKE_BINARY_DIR}/test/tests PROPERTIES DEPENDS)

enable_testing()
add_subdirectory(src)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_subdirectory(bench EXCLUDE_FROM_ALL)
#add_subdirectory(sim EXCLUDE_FROM_ALL)

install(DIRECTORY include/ DESTINATION include)
