aux_source_directory(. DIR_EVALUATOR_SRCS)

add_library(evaluator ${DIR_EVALUATOR_SRCS})
include_directories(../utils/utf)
target_link_libraries(evaluator kafka_client utf)