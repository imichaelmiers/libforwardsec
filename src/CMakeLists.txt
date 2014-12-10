file(GLOB FORWARDSEC_SRCS *.cpp)
file(GLOB FORWARDSEC_INCS *.h)
file(GLOB_RECURSE RELIC_WARPER_SRCS relic_wrapper/*.c*)
file(GLOB_RECURSE RELIC_WARPER_INCS relic_wrapper/*h)

add_library(forwardsec ${FORWARDSEC_SRCS} ${FORWARDSEC_INCS} ${RELIC_WARPER_SRCS}  ${RELIC_WARPER_INCS})
target_include_directories (forwardsec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(forwardsec LINK_PUBLIC relic)