
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_list test_list.c)

target_link_libraries(gp_list base)

