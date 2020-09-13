cmake_minimum_required(VERSION 3.14)

# так пишутся комментарии

project(ermy)

include_directories("include/")

add_library(ermy SHARED source/ermy.cpp)

add_executable(editor source/editor/editor.cpp)
target_link_libraries(editor ermy)