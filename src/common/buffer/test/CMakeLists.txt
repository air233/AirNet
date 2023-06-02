cmake_minimum_required(VERSION 3.12)

project(buffer_test)

set(LIB_DIR "../../../../3rd/lib")

get_filename_component(PARENT_HEAD_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

link_directories(${LIB_DIR}/googletest/lib)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
${PARENT_HEAD_DIR} 
${LIB_DIR}/googletest/include 
"../../until"
)

add_executable(buffer_test 
"../buffer.cpp" 
"../../until/encoding.cpp"
"main.cpp"
"test_buff.cpp"
"test_ringbuff.cpp"
)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(buffer_test gtest)
else()
	#TODO:Á´½Ólibgtest
	
	#target_link_libraries(Buffer libgtest)
endif()

