aux_source_directory(. DIR_DATABASE_SRCS)
# mysqlcppconn8的安装目录
include_directories(/usr/include/mysql-cppconn-8)
include_directories(/usr/include/mysql)

add_link_options(-lssl -lcrypto -lpthread )

add_library(database ${DIR_DATABASE_SRCS})
target_link_libraries(database mysqlcppconn8)


