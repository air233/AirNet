cmake_minimum_required(VERSION 3.12)  # 指定CMake的最低版本要求

project(server_test)  # 设置项目名称

include_directories(
../../
../../net/socket/
../../net/poller/
../../common/log/
../../common/until/
)


# 添加可执行文件
add_executable(server_test  
"test_network.cpp"
"../../net/network.cpp"
# socket文件
"../../net/socket/basesocket.cpp"
"../../net/socket/kcpsocket.cpp"
"../../net/socket/tcpsocket.cpp"
"../../net/socket/udpsocket.cpp"
# 日志文件
"../../common/log/filestream.cpp"
"../../common/log/log.cpp"
# 辅助文件
"../../common/until/times.cpp"

)  # 指定可执行文件名和源文件

# 链接库文件
if(MSVC)
	target_link_libraries(server_test ws2_32.lib)
else()
	#链接libgtest
endif()
