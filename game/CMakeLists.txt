project(game)

find_package(Lua52 REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

file(GLOB_RECURSE game_sources *.cpp)
add_executable(game ${game_sources})
target_link_libraries(game SANIC ${LUA_LIBRARIES})

set_property(TARGET game PROPERTY CXX_STANDARD 11)