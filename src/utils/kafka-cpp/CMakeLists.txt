set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_KAFKA_CLIENT_SRCS)

add_library(kafka_client ${DIR_KAFKA_CLIENT_SRCS})