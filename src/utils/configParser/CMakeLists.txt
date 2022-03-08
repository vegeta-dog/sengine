set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_CONFIGPARSER_SRCS)



add_library(configParser ${DIR_CONFIGPARSER_SRCS})