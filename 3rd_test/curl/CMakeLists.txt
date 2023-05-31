cmake_minimum_required(VERSION 3.12)

project(curl)

add_definitions(-DCURL_STATICLIB)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../3rd/lib/curl/include)

link_directories(../../3rd/lib/curl/lib ../../3rd/lib/openssl/lib ../../3rd/lib/zlib/lib)

add_executable(main main.cpp)


if(MSVC)
	target_link_libraries(main libcurl_a_debug winmm.lib advapi32.lib crypt32.lib normaliz.lib ws2_32.lib wldap32.lib zlibstaticd.lib libcrypto_static.lib libssl_static.lib)
else()
	#Á´½Ólibgtest
endif()

