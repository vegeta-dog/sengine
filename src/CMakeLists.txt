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

set(install_bin_debug ${CMAKE_SOURCE_DIR}/bin/Debug/)
set(install_bin_release ${CMAKE_SOURCE_DIR}/bin/Release/)




# 将当前目录下的代码添加到DIR_SRCS变量
aux_source_directory(. DIR_SRCS)

include_directories(/usr/include)
include_directories(/usr/local/include)
include_directories(./database)
include_directories(./evaluator)
include_directories(./indexBuilder)
include_directories(./libs)
include_directories(./searcher)
include_directories(./utils)

link_directories(/usr/lib)
link_directories(/usr/local/lib)

add_subdirectory(database)
add_subdirectory(evaluator)
add_subdirectory(indexBuilder)
add_subdirectory(libs)
add_subdirectory(searcher)
add_subdirectory(utils)

add_link_options( -lrt -lpthread  -lstdc++ -lboost_filesystem)

# 链接rdkafka需要的库
add_link_options( -lssl -lz -ldl )

set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

set(Boost_USE_STATIC_LIBS   ON)
FIND_PACKAGE(Boost COMPONENTS system filesystem program_options thread json serialization REQUIRED )
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

# 引用的开源库
target_link_libraries(evaluator Boost::json)
target_link_libraries(indexBuilder Boost::json)
target_link_libraries(indexBuilder Boost::serialization)
target_link_libraries(indexBuilder bundle)

target_link_libraries(sEngine ${Boost_LIBRARIES})
target_link_libraries(sEngine spdlog::spdlog)
target_link_libraries(sEngine hiredis)
target_link_libraries(sEngine libssl.a sasl2 crypto librdkafka.a z libzstd.a )
# target_link_libraries(sEngine crypto -static-libgcc -static-libstdc++ librdkafka++.a librdkafka.a z libzstd.a  )

target_link_libraries(sEngine kafka_client)
target_link_libraries(sEngine database)
target_link_libraries(sEngine logger)
target_link_libraries(sEngine configParser)
target_link_libraries(sEngine evaluator)
target_link_libraries(sEngine indexBuilder)
target_link_libraries(sEngine searcher)

# 获取爬虫文件夹下的所有文件
file(GLOB crawlerFiles "${CMAKE_CURRENT_SOURCE_DIR}/se_crawler_dir")
# 分词模块文件夹下的文件
file(GLOB wordSplitFiles "${CMAKE_CURRENT_SOURCE_DIR}/wordSplit")
# webapi下的文件
file(GLOB webapiFiles "${CMAKE_CURRENT_SOURCE_DIR}/webapi")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(TARGETS sEngine DESTINATION ${install_bin_debug})
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/se_crawler_dir/ DESTINATION ${install_bin_debug}/se_crawler_dir)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/wordSplit/ DESTINATION ${install_bin_debug}/wordSplit)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/webapi/ DESTINATION ${install_bin_debug}/webapi)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/utils/kafka_py/ DESTINATION ${install_bin_debug}/utils/kafka_py)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/utils/exception/ DESTINATION ${install_bin_debug}/utils/exception)

    install(FILES ${PROJECT_SOURCE_DIR}/requirements.txt DESTINATION ${install_bin_debug}/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/__init__.py DESTINATION ${install_bin_debug}/utils/)

    install(FILES ${PROJECT_SOURCE_DIR}/utils/configParser/__init__.py DESTINATION ${install_bin_debug}/utils/configParser/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/configParser/configParser.py DESTINATION ${install_bin_debug}/utils/configParser/)

    install(FILES ${PROJECT_SOURCE_DIR}/utils/logger/log.py DESTINATION ${install_bin_debug}/utils/logger/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/logger/__init__.py DESTINATION ${install_bin_debug}/utils/logger/)

    # file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug/config.ini DESTINATION "${CMAKE_SOURCE_DIR}/bin/Debug/")
else ()
    install(TARGETS sEngine DESTINATION ${install_bin_release})

    install(TARGETS sEngine DESTINATION ${install_bin_release})
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/se_crawler_dir/ DESTINATION ${install_bin_release}/se_crawler_dir)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/wordSplit/ DESTINATION ${install_bin_release}/wordSplit)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/webapi/ DESTINATION ${install_bin_release}/webapi)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/utils/kafka_py/ DESTINATION ${install_bin_release}/utils/kafka_py)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/utils/exception/ DESTINATION ${install_bin_release}/utils/exception)

    install(FILES ${PROJECT_SOURCE_DIR}/requirements.txt DESTINATION ${install_bin_release}/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/__init__.py DESTINATION ${install_bin_release}/utils/)

    install(FILES ${PROJECT_SOURCE_DIR}/utils/configParser/__init__.py DESTINATION ${install_bin_release}/utils/configParser/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/configParser/configParser.py DESTINATION ${install_bin_release}/utils/configParser/)
    
    install(FILES ${PROJECT_SOURCE_DIR}/utils/logger/log.py DESTINATION ${install_bin_release}/utils/logger/)
    install(FILES ${PROJECT_SOURCE_DIR}/utils/logger/__init__.py DESTINATION ${install_bin_release}/utils/logger/)
    # file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-release/config.ini DESTINATION "${CMAKE_SOURCE_DIR}/bin/Release/")
endif ()
