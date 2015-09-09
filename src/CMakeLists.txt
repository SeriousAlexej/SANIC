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
set_target_properties(lBulletDynamics PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletDynamics/libBulletDynamics.a)

add_library(lBulletCollision SHARED IMPORTED)
set_target_properties(lBulletCollision PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/BulletCollision/libBulletCollision.a)

add_library(lLinearMath SHARED IMPORTED)
set_target_properties(lLinearMath PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/bullet/src/LinearMath/libLinearMath.a)

add_library(lAntTweakBar SHARED IMPORTED)
set_target_properties(lAntTweakBar PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/lib/libAntTweakBar.so)

add_library(lLuaCpp STATIC IMPORTED)
set_target_properties(lLuaCpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/lib/libluacppinterface.a)


add_executable(SANIC ${SANIC_sources} ${TEST_sources})
set_property(TARGET SANIC PROPERTY CXX_STANDARD 11)

target_link_libraries(SANIC lBulletDynamics lBulletCollision lLinearMath lAntTweakBar lLuaCpp ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${Boost_LIBRARIES} ${SFML_LIBRARIES} ${SFGUI_LIBRARY} ${LUA_LIBRARIES})

