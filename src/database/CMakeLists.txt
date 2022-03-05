aux_source_directory(. DIR_DATABASE_SRCS)
# mysqlcppconn8的安装目录
include_directories(/usr/include/mysql-cppconn-8)

link_libraries(mysqlcppconn8)

add_library(database ${DIR_DATABASE_SRCS})
