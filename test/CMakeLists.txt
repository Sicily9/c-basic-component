if(test)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)
	set(list_src "list/test_list.cpp" "list/sds.h" "list/sds.c" "test.h")
	set(dict_src "dict/test_dict.cpp" "dict/sds.h" "dict/sds.c" "test.h")
	set(loop_src "loop/test_loop.c") 
	set(server_src "server/guish_main.c"
			"server/proto_interface/cwaf_config_buf.pb-c.h"
			"server/proto_interface/cwaf_config_buf.pb-c.c" 
			"server/guish_server.h" "server/guish_server.c"
			"server/proto_handler/dns_config_handler.c"
			"server/proto_handler/static_route_handler.c"
			"server/proto_handler/default_gateway_handler.c"
			) 
	set(client_dns_config_src "client/test_client_dns_config.c" 
			"client/proto_interface/cwaf_config_buf.pb-c.h"
			"client/proto_interface/cwaf_config_buf.pb-c.c"
			) 
	set(client_static_route_src "client/test_client_static_route.c" 
			"client/proto_interface/cwaf_config_buf.pb-c.h"
			"client/proto_interface/cwaf_config_buf.pb-c.c"
			) 
	set(client_default_gateway_src "client/test_client_default_gateway.c" 
			"client/proto_interface/cwaf_config_buf.pb-c.h"
			"client/proto_interface/cwaf_config_buf.pb-c.c"
			) 
	set(module_src "module/test_module.cpp")
	set(thread_src "thread/test_thread.c") 
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
	#build_target(T "gp_loop" SRC ${loop_src} LIBS ${lib_dep})
	build_target(T "gp_server" SRC ${server_src} LIBS ${lib2_dep})
	build_target(T "gp_client_dns_config" SRC ${client_dns_config_src} LIBS ${lib2_dep})
	build_target(T "gp_client_static_route" SRC ${client_static_route_src} LIBS ${lib2_dep})
	build_target(T "gp_client_default_gateway" SRC ${client_default_gateway_src} LIBS ${lib2_dep})

	#add_custom_command(TARGET gp_thread
	#		           POST_BUILD
	#			       COMMAND make test
	#)

endif()
