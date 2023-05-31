cmake_minimum_required(VERSION 3.12)

project(main)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../3rd/lib/glog/include)

link_directories(../../3rd/lib/glog/lib)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE glogd.lib)