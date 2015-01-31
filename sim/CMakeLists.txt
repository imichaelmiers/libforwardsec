#BOOST dependencies for serialization.
find_package(Boost 1.5 COMPONENTS timer system)
if(Boost_TIMER_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
else()
	message(FATAL_ERROR, "Cannot find boost timer library")
endif(Boost_TIMER_FOUND)


add_executable(sim sim.cpp)
target_link_libraries (sim LINK_PUBLIC forwardsec  ${Boost_TIMER_LIBRARY} ${Boost_SYSTEM_LIBRARY})
add_dependencies(sim forwardsec)
