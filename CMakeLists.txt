cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include_directories(${CMAKE_SOURCE_DIR}/bullet/src/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps/include/)
INCLUDE_DIRECTORIES(/usr/include/lua5.2/)

file(GLOB_RECURSE SANIC_sources src/*.cpp src/*.c src/*.cc)
#file(GLOB_RECURSE TEST_sources test/*.cpp)

add_subdirectory(src src)