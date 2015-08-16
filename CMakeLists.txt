cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/bullet/src/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/deps/include/)

add_subdirectory(test test)
add_subdirectory(src src)
