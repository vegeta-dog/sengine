aux_source_directory(. DIR_DATABASE_SRCS)
# mysqlcppconn8的安装目录
set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
include_directories(/usr/include/mysql)
include_directories(/usr/local/include/hiredis)
link_directories(/usr/local/lib)
add_link_options(-lssl -lcrypto -lpthread -llibhiredis)

add_library(database ${DIR_DATABASE_SRCS})
target_link_libraries(database mysqlclient)


