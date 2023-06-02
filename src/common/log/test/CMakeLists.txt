cmake_minimum_required(VERSION 3.12)

project(log_test)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PARENT_DIR} ../../until/times.h)

add_executable(log_test 
"../log.cpp" 
"../../until/times.cpp"
"../logstream.cpp"
"test_log.cpp"
)