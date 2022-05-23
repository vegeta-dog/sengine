set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(. DIR_ENCODING_SRCS)

add_subdirectory(utf)
add_subdirectory(base64)
include_directories(./utf)

