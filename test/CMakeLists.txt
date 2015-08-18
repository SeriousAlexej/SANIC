project(tests)

find_package(Lua52 REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

file(GLOB_RECURSE test_sources *.cpp)
add_executable(tests ${test_sources})
target_link_libraries(tests SANIC ${LUA_LIBRARIES})

set_property(TARGET tests PROPERTY CXX_STANDARD 11)

add_test(NAME tests COMMAND tests)
