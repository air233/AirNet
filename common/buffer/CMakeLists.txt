cmake_minimum_required(VERSION 3.12)

project(Buffer)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR} 
../until
../../3rd/lib/googletest/include 
)

link_directories(../../3rd/lib/googletest/lib)

file(GLOB SOURCES "../until/encoding.cpp")

add_executable(main main.cpp buffer.cpp ${SOURCES} "test/test_buff.cpp" "test/test_ringbuff.cpp")

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	target_link_libraries(main gtest)

else()
	#Á´½Ólibgtest
	target_link_libraries(main libgtest)
endif()

