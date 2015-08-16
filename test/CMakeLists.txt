project(tests)

file(GLOB_RECURSE test_sources *.cpp)
add_executable(tests ${test_sources})
target_link_libraries(tests SANIC)

set_property(TARGET tests PROPERTY CXX_STANDARD 11)

add_test(NAME tests COMMAND tests)
