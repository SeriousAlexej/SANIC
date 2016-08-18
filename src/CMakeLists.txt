cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

include_directories(${CMAKE_SOURCE_DIR}/../bullet/src/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../deps/include/)
INCLUDE_DIRECTORIES(/usr/include/lua5.2/)

file(GLOB_RECURSE SANIC_sources *.cpp *.c *.cc)
#file(GLOB_RECURSE TEST_sources test/*.cpp)

project(SANIC)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(SFML COMPONENTS system window graphics REQUIRED)
find_package(Lua52 REQUIRED)
find_package(SFGUI REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
include_directories(${SFGUI_INCLUDE_DIR})

add_library(lBulletDynamics SHARED IMPORTED)
set_target_properties(lBulletDynamics PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../bullet/src/BulletDynamics/libBulletDynamics.a)

add_library(lBulletCollision SHARED IMPORTED)
set_target_properties(lBulletCollision PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../bullet/src/BulletCollision/libBulletCollision.a)

add_library(lLinearMath SHARED IMPORTED)
set_target_properties(lLinearMath PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../bullet/src/LinearMath/libLinearMath.a)

add_library(lLuaCpp STATIC IMPORTED)
set_target_properties(lLuaCpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../deps/lib/libluacppinterface.a)


add_executable(EGGINE ${SANIC_sources} ${TEST_sources})
set_property(TARGET EGGINE PROPERTY CXX_STANDARD 14)

target_link_libraries(EGGINE lBulletDynamics lBulletCollision lLinearMath lLuaCpp ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Boost_LIBRARIES} ${SFML_LIBRARIES} ${SFGUI_LIBRARY} ${LUA_LIBRARIES})

