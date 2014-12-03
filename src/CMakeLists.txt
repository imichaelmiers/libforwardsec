file(GLOB FORWARDSEC_SRCS *.c)
file(GLOB FORWARDSEC_INCS *.h)
add_library(forwardsec ${FORWARDSEC_SRCS} ${FORWARDSEC_INCS})
target_include_directories (forwardsec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
