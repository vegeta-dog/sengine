set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_BASE64_SRCS)

include_directories(.)

add_library(libBase64 ${DIR_BASE64_SRCS})