cmake_minimum_required (VERSION 2.6)
project (RTGDE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/include")
add_definitions(-Wall)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

find_package(Threads REQUIRED)

file(GLOB rtgde_SOURCES "lib/*.c" "lib/predictions/*.c" "lib/models/*.c"
			"lib/*.h" "include/*.h" "include/predictions/*.h" "include/models/*.h")

add_library(rtgde SHARED ${rtgde_SOURCES})
set_property(TARGET rtgde PROPERTY VERSION 0.1.0)
target_link_libraries(rtgde ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_graph tests/graph.c)
target_link_libraries(test_graph rtgde)

add_executable(test_flowgraph tests/flowgraph.c)
target_link_libraries(test_flowgraph rtgde)

find_package(PCAP)
if(PCAP_FOUND)
	add_executable(test_pcap_live tests/pcap_live.c)
	link_directories(${PCAP_INCLUDE_DIR})
	target_link_libraries(test_pcap_live rtgde ${PCAP_LIBRARY})
	set(PROG_PCAP_LIVE "Yes")
else(PCAP_FOUND)
	set(PROG_PCAP_LIVE "No")
endif(PCAP_FOUND)

install(TARGETS rtgde DESTINATION "lib/")
install(FILES "${CMAKE_BIN_DIR}/include" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rtgde/")

#Messages
message("")
message("-- Summary:")
message("* Compilation type: CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("* Install directory: CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("")
message("-- Tests:")
message("* pcap_live: ${PROG_PCAP_LIVE} (requires libpcap)")
message("")
