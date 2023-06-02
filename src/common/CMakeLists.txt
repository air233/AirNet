cmake_minimum_required(VERSION 3.0)

project(ANet)

# 设置 NetBuild 的 include 目录的路径
set(NETBUILD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../anet/include/common)
# 设置 NetBuild 的 lib 目录的路径
set(NETBUILD_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../anet/lib)

# 设置 common 目录的路径
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# 获取 common 目录下的所有子目录
file(GLOB SUBDIRECTORIES ${COMMON_DIR}/*)
# 遍历每个子目录
foreach(SUBDIRECTORY ${SUBDIRECTORIES})
	# 检查是否为文件夹
	if(IS_DIRECTORY ${SUBDIRECTORY})
		# 排除 build,out,.vs目录
		get_filename_component(SUBDIRECTORY_NAME ${SUBDIRECTORY} NAME)
		if(NOT (SUBDIRECTORY_NAME STREQUAL "build" OR SUBDIRECTORY_NAME STREQUAL "out" OR SUBDIRECTORY_NAME STREQUAL ".vs"))
			# 构建当前子目录的绝对路径
			set(FULL_PATH ${COMMON_DIR}/${SUBDIRECTORY_NAME})

			# 获取当前子目录下的所有 .h 文件
			file(GLOB CURRENT_HEADERS ${FULL_PATH}/*.h)
			#message("CURRENT_HEADERS: ${CURRENT_HEADERS}")
			
			file(GLOB CURRENT_CPP_SOURCE ${FULL_PATH}/*.cpp)
			file(GLOB CURRENT_C_SOURCE ${FULL_PATH}/*.c)

			list(APPEND SOURCE_FILES ${CURRENT_CPP_SOURCE})
			list(APPEND SOURCE_FILES ${CURRENT_C_SOURCE})

			# 安装当前子目录的 .h 文件到 NetBuild 的 include 目录下
			#install(FILES ${CURRENT_HEADERS} DESTINATION ${NETBUILD_INCLUDE_DIR}/${SUBDIRECTORY_NAME})
			file(COPY ${CURRENT_HEADERS} DESTINATION ${NETBUILD_INCLUDE_DIR}/${SUBDIRECTORY_NAME})
		endif()
	endif()
endforeach()

# 设置 3rd 目录路径
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../3rd/lib/)
# 获取 3rd 目录下所有子目录
file(GLOB LIB_DIRS LIST_DIRECTORIES TRUE ${THIRD_PARTY_DIR}/*)
# 遍历每个子目录，获取目录名
foreach(LIB_DIR ${LIB_DIRS})
    get_filename_component(LIB_NAME ${LIB_DIR} NAME)
    #message("LIB_NAME: ${LIB_NAME}")
	# 设置库的链接路径
	#list(APPEND THIRD_PARTY_LIB_DIR ${THIRD_PARTY_DIR}${LIB_NAME}/lib)
	list(APPEND THIRD_PARTY_INC_DIR ${THIRD_PARTY_DIR}${LIB_NAME}/include)
endforeach()


add_library(anetcommon ${SOURCE_FILES})

include_directories(${THIRD_PARTY_INC_DIR})

#链接3rd目录下的库:
if(MSVC)
	set(THIRD_PARTY_LIB_DIR
		${THIRD_PARTY_LIB_DIR}/openssl/lib
	)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	target_link_libraries(anetcommon ${THIRD_PARTY_LIB_DIR})
else()

endif()

#install(TARGETS anetcommon DESTINATION ${NETBUILD_LIB_DIR})
add_custom_command(TARGET anetcommon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:anetcommon> ${NETBUILD_LIB_DIR}
)
