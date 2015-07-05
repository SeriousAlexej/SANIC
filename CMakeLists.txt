cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(SANIC)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(SFML COMPONENTS system window graphics REQUIRED)

add_library(lBulletDynamics SHARED IMPORTED)
set_target_properties(lBulletDynamics PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletDynamics/libBulletDynamics.a)

add_library(lBulletCollision SHARED IMPORTED)
set_target_properties(lBulletCollision PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletCollision/libBulletCollision.a)

add_library(lLinearMath SHARED IMPORTED)
set_target_properties(lLinearMath PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/LinearMath/libLinearMath.a)

add_library(lAntTweakBar SHARED IMPORTED)
set_target_properties(lAntTweakBar PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/lib/libAntTweakBar.so)

include_directories(${CMAKE_SOURCE_DIR}/bullet/src/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/deps/include/)

file(GLOB_RECURSE SANIC_sources ./src/*.cpp ./src/*.c ./src/*.cc)

add_executable(SANIC ${SANIC_sources})
set_property(TARGET SANIC PROPERTY CXX_STANDARD 11)

target_link_libraries(SANIC lBulletDynamics lBulletCollision lLinearMath lAntTweakBar ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Boost_LIBRARIES} ${SFML_LIBRARIES})


