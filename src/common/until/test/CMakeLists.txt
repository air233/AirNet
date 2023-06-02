cmake_minimum_required(VERSION 3.12)

project(until_test)

set(LIB_DIR "../../../../3rd/lib")

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
${PARENT_DIR} 
${LIB_DIR}/googletest/include
${LIB_DIR}/openssl/include
)

link_directories(${LIB_DIR}/googletest/lib ${LIB_DIR}/openssl/lib)

add_executable(until_test  "../encoding.cpp" "test_encoding.cpp" "../base64.h" "../base64.cpp" "../md5.h" "../md5.cpp" "test_md5.cpp" "test_base64.cpp" "main.cpp" "../times.cpp" "test_times.cpp")

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(until_test gtest libcrypto_static libssl_static)

else()
	#Á´½Ólibgtest
	target_link_libraries(until_test gtest libcrypto_static libssl_static)
endif()