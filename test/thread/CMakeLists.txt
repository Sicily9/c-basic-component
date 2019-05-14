
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_thread test_thread.c)


target_link_libraries(gp_thread base)

