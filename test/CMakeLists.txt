cmake_minimum_required(VERSION 3.12)

project(network_test)

set(LIB_DIR "../3rd/lib")
set(COMMON_DIR "../src/common/")
set(NET_DIR "../src/net/")
#get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
#${PARENT_DIR}
${COMMON_DIR}
${NET_DIR}
#${LIB_DIR}/googletest/include
)

#link_directories(${LIB_DIR}/googletest/lib)

add_executable(network_test

"${COMMON_DIR}/until/encoding.cpp"
"${COMMON_DIR}/until/times.cpp"
"${COMMON_DIR}/log/log.cpp"
"${COMMON_DIR}/log/filestream.cpp"
"${COMMON_DIR}/buffer/buffer.cpp"
"${NET_DIR}/network/network.cpp"
"${NET_DIR}/socket/socketops.cpp"
"${NET_DIR}/poller/poll.cpp"
"${NET_DIR}/poller/iocp.cpp"
"${NET_DIR}/poller/epoll.cpp"
"${NET_DIR}/netobj/basenetobj.cpp"
"${NET_DIR}/netobj/tcpnetobj.cpp"
"${NET_DIR}/until/inetAddress.cpp"
"${NET_DIR}/inetwork.cpp"

#"${PARENT_DIR}/inetAddress.cpp"
#"${COMMON_DIR}/until/encoding.cpp" 
#"${COMMON_DIR}/until/times.cpp" 
#"main.cpp" 
"test_network.cpp"
)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(network_test ws2_32.lib)
else()
	#Á´½Ólibgtest
endif()

