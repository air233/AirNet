cmake_minimum_required(VERSION 3.12)

project(netlib_test)

link_directories(../lib)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
../include/
)

add_executable(netlib_test 
main.cpp
)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	target_link_libraries(netlib_test anetcommon.lib)
else()
	#TODO:链接libgtest
	
	#target_link_libraries(Buffer libgtest)
endif()

#将编译后的程序输出到bin目录
set_target_properties(netlib_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin
)
