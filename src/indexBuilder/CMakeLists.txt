aux_source_directory(. DIR_INDEX_BUILDER_SRCS)

add_library(indexBuilder ${DIR_INDEX_BUILDER_SRCS})
target_link_libraries(indexBuilder kafka_client)