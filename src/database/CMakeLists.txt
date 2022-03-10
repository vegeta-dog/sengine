aux_source_directory(. DIR_DATABASE_SRCS)

# 获取sql文件列表
file(GLOB SQL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sql/*.sql)

# mysqlcppconn8的安装目录
set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
include_directories(/usr/include/mysql)
include_directories(/usr/local/include/hiredis)
link_directories(/usr/local/lib)
add_link_options(-lssl -lcrypto -lpthread -llibhiredis)

add_library(database ${DIR_DATABASE_SRCS})
target_link_libraries(database mysqlclient)
message("SQL files: ${SQL_FILES}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    foreach(f ${SQL_FILES})
        install(FILES ${f} DESTINATION "${install_bin_debug}/database/sql")
        file(COPY ${f} DESTINATION "${publish_bin_debug}/database/sql")
    endforeach()

else ()
    foreach(f ${SQL_FILES})
        install(${f} DESTINATION "${install_bin_release}/database/sql")
        file(COPY ${f} DESTINATION "${publish_bin_release}/database/sql")
        message("writing to ${publish_bin_release}/database/sql")
    endforeach()
endif ()
