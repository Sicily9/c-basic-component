
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

add_executable(gp_threadpool test_threadpool.c task.pb-c.h task.pb-c.c proto.h
	proto.c)


target_link_libraries(gp_threadpool base)
target_link_libraries(gp_threadpool protobuf-c)

