if(test)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)
	set(list_src "list/test_list.cpp" "list/sds.h" "list/sds.c" "test.h")
	set(dict_src "dict/test_dict.cpp" "dict/sds.h" "dict/sds.c" "test.h")
	set(loop_src "loop/test_loop.c") 
	set(server_src "server/test_server.c") 
	set(module_src "module/test_module.cpp")
	set(thread_src "thread/test_thread.c") 
	set(threadpool_src "threadpool/test_threadpool.c" "threadpool/task.pb-c.h"
			"threadpool/task.pb-c.c" "threadpool/proto.h" "threadpool/proto.c") 
	set(yaml_config_src "yaml_config/test_yaml_config.cpp") 
	set(lib_dep "base" "gtest" "gmock") 
	set(lib2_dep "base" "gtest" "gmock" "protobuf-c")

	#build_test_target in top dir cmakelists.txt
	build_target(T "gp_yaml_config" SRC ${yaml_config_src} LIBS ${lib_dep})
	#gtest_discover_tests(gp_yaml_config)
	build_target(T "gp_list" SRC ${list_src} LIBS ${lib_dep})
	#gtest_discover_tests(gp_list)
	build_target(T "gp_dict" SRC ${dict_src} LIBS ${lib_dep})
	#gtest_discover_tests(gp_dict)
	build_target(T "gp_module" SRC ${module_src} LIBS ${lib_dep})
	#gtest_discover_tests(gp_module)
	build_target(T "gp_thread" SRC ${thread_src} LIBS ${lib_dep})
	build_target(T "gp_threadpool" SRC ${threadpool_src} LIBS ${lib2_dep})
	#build_target(T "gp_loop" SRC ${loop_src} LIBS ${lib_dep})
	build_target(T "gp_server" SRC ${server_src} LIBS ${lib_dep})

	add_custom_command(TARGET gp_threadpool
			           POST_BUILD
				       COMMAND make test
	)

endif()
