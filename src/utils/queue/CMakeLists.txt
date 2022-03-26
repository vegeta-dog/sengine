set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_QUEUE_SRCS)

add_library(queue ${DIR_QUEUE_SRCS})