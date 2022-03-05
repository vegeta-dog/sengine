cmake_minimum_required(VERSION 3.20)
project(sEngine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 将当前目录下的代码添加到DIR_SRCS变量
aux_source_directory(. DIR_SRCS)

include_directories(./database)
include_directories(./utils)
include_directories(./libs)

add_subdirectory(database)
add_subdirectory(utils)
add_subdirectory(libs)

add_executable(sEngine ${DIR_SRCS})

target_link_libraries(sEngine mysqlcppconn8)
target_link_libraries(sEngine database)
target_link_libraries(sEngine configParser)

# 引用的开源库
target_link_libraries(sEngine EasyLogger)

install(TARGETS sEngine DESTINATION "${CMAKE_SOURCE_DIR}/bin/")
