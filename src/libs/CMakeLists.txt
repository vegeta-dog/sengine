set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(kafka)
include_directories(bundle)
add_subdirectory(kafka)
add_subdirectory(bundle)

# 将当前目录下的代码添加到DIR_SRCS变量
aux_source_directory(. DIR_SRCS)

