cmake_minimum_required(VERSION 3.10)
project(clipboard)

add_library(clipboard STATIC clipboard.c)

add_executable(demo demo.c)
target_link_libraries(demo clipboard X11 Xfixes)
