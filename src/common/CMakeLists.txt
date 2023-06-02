#set(SRCS buffer/*.cpp)

#add_library(ANet ${SRCS})

#install(TARGETS ANet DESTINATION NetBuild/lib)

cmake_minimum_required(VERSION 3.0)
project(ANet)


# 设置 common 目录的路径
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置 NetBuild 的 include 目录的路径
set(NETBUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../anet/include)

# 获取 common 目录下的所有子目录
file(GLOB SUBDIRECTORIES ${COMMON_DIR}/*)

# 遍历每个子目录
foreach(SUBDIRECTORY ${SUBDIRECTORIES})
	# 检查是否为文件夹
	if(IS_DIRECTORY ${SUBDIRECTORY})
		# 排除 build,out,.vs目录
		get_filename_component(SUBDIRECTORY_NAME ${SUBDIRECTORY} NAME)
		if(NOT (SUBDIRECTORY_NAME STREQUAL "build" OR SUBDIRECTORY_NAME STREQUAL "out" OR SUBDIRECTORY_NAME STREQUAL ".vs"))
			message("SUBDIRECTORY_NAME: ${SUBDIRECTORY_NAME}")
			# 构建当前子目录的绝对路径
			set(FULL_PATH ${COMMON_DIR}/${SUBDIRECTORY_NAME})
			message("Full: ${FULL_PATH}")

			# 获取当前子目录下的所有 .h 文件
			file(GLOB CURRENT_HEADERS ${FULL_PATH}/*.h)
			message("CURRENT_HEADERS: ${CURRENT_HEADERS}")
			
			# 安装当前子目录的 .h 文件到 NetBuild 的 include 目录下
			install(FILES ${CURRENT_HEADERS} DESTINATION ${NETBUILD_INCLUDE_DIR}/${SUBDIRECTORY_NAME})
			message("CURRENT_HEADERS: ${CURRENT_HEADERS}")
			message("Target: ${NETBUILD_INCLUDE_DIR}/${SUBDIRECTORY_NAME}")

		endif()
	endif()
endforeach()
