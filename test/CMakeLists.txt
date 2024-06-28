#设置编译源文件
aux_source_directory(. SRC_LIST)

#生成可执行文件
add_executable(test ${SRC_LIST})

# 添加库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

#链接时依赖的库文件
target_link_libraries(test dbconnectionpool pthread)