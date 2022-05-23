
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 将当前目录下的代码添加到DIR_SRCS变量
aux_source_directory(. DIR_SRCS)

include_directories(./configParser)
include_directories(./logger)
include_directories(./kafka-cpp)
include_directories(./encoding)

add_subdirectory(configParser)
add_subdirectory(logger)
add_subdirectory(encoding)
add_subdirectory(kafka-cpp)
