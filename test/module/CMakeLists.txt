
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_module test_module.c)


target_link_libraries(gp_module base)

