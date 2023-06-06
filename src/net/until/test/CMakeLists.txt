cmake_minimum_required(VERSION 3.12)

project(addres_test)

set(LIB_DIR "../../../../3rd/lib")
set(COMMON_DIR "../../../common/")

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
${PARENT_DIR}
${COMMON_DIR}
"../../../"
${LIB_DIR}/googletest/include
)

link_directories(${LIB_DIR}/googletest/lib)

add_executable(addres_test
"${PARENT_DIR}/InetAddress.cpp"
"${COMMON_DIR}/until/encoding.cpp" 
"${COMMON_DIR}/until/times.cpp" 
"${COMMON_DIR}/log/log.cpp"
"${COMMON_DIR}/log/filestream.cpp"
"main.cpp" 
"test_inetaddress.cpp"
)

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(addres_test gtest ws2_32.lib)

	#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
	#Á´½Ólibgtest
endif()

