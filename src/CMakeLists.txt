cmake_minimum_required(VERSION 3.18)
project(sEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# 设置 可执行程序输出目录
set(publish_bin_debug ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)
set(publish_bin_release ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-release)

# 设置库文件输出目录
set(publish_lib_debug ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)
set(publish_lib_release ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-release)

add_link_options(-lrt -pthread )
add_compile_options(-std=c++17 -lstdc++)

# 将当前目录下的代码添加到DIR_SRCS变量
aux_source_directory(. DIR_SRCS)

include_directories(./database)
include_directories(./utils)
include_directories(./libs)

add_subdirectory(database)
add_subdirectory(utils)
add_subdirectory(libs)


FIND_PACKAGE(Boost COMPONENTS  system filesystem thread REQUIRED )
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
else ()
    message("Can not found BOOST")
endif ()

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()



add_executable(sEngine ${DIR_SRCS})

target_link_libraries(sEngine ${Boost_LIBRARIES})
target_link_libraries(sEngine spdlog::spdlog)


target_link_libraries(sEngine mysqlcppconn8)


target_link_libraries(sEngine database)
target_link_libraries(sEngine configParser)
target_link_libraries(sEngine logger)
# 引用的开源库


if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(TARGETS sEngine DESTINATION "${CMAKE_SOURCE_DIR}/bin/Debug/")
else ()
    install(TARGETS sEngine DESTINATION "${CMAKE_SOURCE_DIR}/bin/Release/")
endif ()
