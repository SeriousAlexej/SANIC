cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/bullet/src/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps/include/)
INCLUDE_DIRECTORIES(/usr/include/lua5.2/)

add_subdirectory(test test)
add_subdirectory(src src)
add_subdirectory(game game)