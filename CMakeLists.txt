cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(SANIC)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(SFML COMPONENTS system window graphics REQUIRED)
find_package(AntTweakBar REQUIRED)

add_library(lBulletDynamics SHARED IMPORTED)
set_target_properties(lBulletDynamics PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletDynamics/libBulletDynamics.a)

add_library(lBulletCollision SHARED IMPORTED)
set_target_properties(lBulletCollision PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletCollision/libBulletCollision.a)

add_library(lLinearMath SHARED IMPORTED)
set_target_properties(lLinearMath PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/LinearMath/libLinearMath.a)

include_directories(${CMAKE_SOURCE_DIR}/bullet/src/)
include_directories(${Boost_INCLUDE_DIR})

file(GLOB_RECURSE SANIC_sources ./src/*.cpp ./src/*.c ./src/*.cc)

add_executable(SANIC ${SANIC_sources})

target_link_libraries(SANIC lBulletDynamics lBulletCollision lLinearMath ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Boost_LIBRARIES} ${SFML_LIBRARIES} ${ANT_TWEAK_BAR_LIBRARY})

set_property(TARGET SANIC PROPERTY CXX_STANDARD 11)