set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(. DIR_BUNDLE_SRCS)

add_library(bundle ${DIR_BUNDLE_SRCS})
