cmake_minimum_required(VERSION 3.12)

project(Until)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../3rd/lib/googletest/include ../../3rd/lib/openssl/include)

link_directories(../../3rd/lib/googletest/lib ../../3rd/lib/openssl/lib)

add_executable(until  "encoding.cpp" "test/test_encoding.cpp" "base64.h" "base64.cpp" "md5.h" "md5.cpp" "test/test_md5.cpp" "test/test_base64.cpp" "test/main.cpp" "times.cpp" "test/test_times.cpp")


if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(until gtest libcrypto_static libssl_static)

else()
	#Á´½Ólibgtest
	target_link_libraries(until gtest libcrypto_static libssl_static)
endif()