set(CMAKE_INCLUDE_CURRENT_DIR ON)
aux_source_directory(. DIR_DATABASE_SRCS)


add_library(logger ${DIR_DATABASE_SRCS})

