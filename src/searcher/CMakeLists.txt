aux_source_directory(. DIR_SEARCHER_SRCS)

add_library(searcher ${DIR_SEARCHER_SRCS})
target_link_libraries(searcher kafka_client)