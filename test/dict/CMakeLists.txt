
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_dict test_dict.c sds.h sds.c)


target_link_libraries(gp_dict base)

