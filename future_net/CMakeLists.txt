# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.3)

# 项目信息
project(future_net)

# include路径
include_directories(${PROJECT_SOURCE_DIR}/lib)

# 设置可执行文件生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

# 生成debug版本
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 lib 子目录
#add_subdirectory(lib)

# 指定生成目标 
add_executable(future_net ${DIR_SRCS})

# 生成带有路径的连接库，存储于 FUTURE_NET_LIBRARY 变量
find_library(FUTURE_NET_LIBRARY _io ${PROJECT_BINARY_DIR}/../lib)
IF (NOT FUTURE_NET_LIBRARY)
    MESSAGE(FATAL_ERROR "FUTURE_NET_LIBRARY not found")
ENDIF(NOT FUTURE_NET_LIBRARY)

# 添加链接库
target_link_libraries(future_net ${FUTURE_NET_LIBRARY})
target_link_libraries(future_net pthread)