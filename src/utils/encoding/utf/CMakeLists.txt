set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_UTF_SRCS)



add_library(utf ${DIR_UTF_SRCS})