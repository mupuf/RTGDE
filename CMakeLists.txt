cmake_minimum_required (VERSION 2.6)
project (RTGDE)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/include")
add_definitions(-Wall)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

find_package(Threads REQUIRED)

option(USEPCAP "USEPCAP" ON)
if(USEPCAP)
	find_package(PCAP)
	if(PCAP_FOUND)
		add_definitions(-DHAS_LIBPCAP)
		link_directories(${PCAP_INCLUDE_DIR})
	else(PCAP_FOUND)
		set(USEPCAP OFF)
		set(PCAP_LIBRARY "")
	endif(PCAP_FOUND)
endif(USEPCAP)

file(GLOB_RECURSE rtgde_SOURCES "lib/*.c" "lib/*.h"
				"include/*.h")

add_library(rtgde SHARED ${rtgde_SOURCES})
set_property(TARGET rtgde PROPERTY VERSION 0.1.0)
target_link_libraries(rtgde ${CMAKE_THREAD_LIBS_INIT} -lm)

add_executable(test_graph tests/graph.c)
target_link_libraries(test_graph rtgde)

add_executable(test_flowgraph tests/flowgraph.c)
target_link_libraries(test_flowgraph rtgde)

add_executable(test_pcap tests/pcap/pcap.c)
target_link_libraries(test_pcap rtgde ${PCAP_LIBRARY})

add_executable(test_nv_graph tests/nv_graph.c)
target_link_libraries(test_nv_graph rtgde)

install(TARGETS rtgde DESTINATION "lib/")
install(FILES "${CMAKE_BIN_DIR}/include" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rtgde/")

#Messages
message("")
message("-- Summary:")
message("* Compilation type: CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("* Install directory: CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("")
message("-- Tests:")
message("* pcap live support: ${USEPCAP} (requires libpcap)")
message("")
