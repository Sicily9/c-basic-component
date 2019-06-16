
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_loop test_loop.c)

target_link_libraries(gp_loop base)

