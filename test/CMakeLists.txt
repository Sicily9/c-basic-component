if(test)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)
    file(GLOB server_src 
            "server/*.c" "server/*.h"
            "server/proto_interface/*"
            "server/proto_handler/*.c")
    file(GLOB client_dns_config_src 
            "client/test_client_dns_config.c" 
            "client/proto_interface/*")
    file(GLOB client_static_route_src 
            "client/test_client_static_route.c" 
            "client/proto_interface/*")
    file(GLOB client_default_gateway_src 
            "client/test_client_default_gateway.c"
            "client/proto_interface/*")
    file(GLOB client_manage_service_src 
            "client/test_client_manage_service.c"
            "client/proto_interface/*")
    file(GLOB client_interface_src 
            "client/test_client_interface.c"
            "client/proto_interface/*")
    file(GLOB client_match_option_src 
            "client/test_client_match_option.c"
            "client/proto_interface/*")
	file(GLOB client_define_rule_src 
            "client/test_client_define_rule.c"
            "client/proto_interface/*")
    file(GLOB client_ssl_certificate_src 
            "client/test_client_ssl_certificate.c"
            "client/proto_interface/*")
    file(GLOB client_bizserver_src 
            "client/test_client_bizserver.c"
            "client/proto_interface/*")
    file(GLOB client_attack_detect_src 
            "client/test_client_attack_detect.c"
            "client/proto_interface/*")
    file(GLOB client_site_security_src 
            "client/test_client_site_security.c"
            "client/proto_interface/*")
    file(GLOB client_predef_sign_rule_src 
            "client/test_client_predef_sign_rule.c"
            "client/proto_interface/*")
    file(GLOB client_virtual_service_src 
            "client/test_client_virtual_service.c"
            "client/proto_interface/*")
    file(GLOB list_src 
            "list/*.cpp" "list/*.h" 
            "list/*.c")
    file(GLOB dict_src 
            "dict/*.cpp" "dict/*.h" 
            "dict/*.c")
    set(module_src "module/test_module.cpp")
    set(thread_src "thread/test_thread.c") 
    set(yaml_config_src "yaml_config/test_yaml_config.cpp") 
    set(lib_dep "base" "gtest" "gmock") 
    set(lib2_dep "base" "gtest" "gmock")
    set(lib3_dep "base")

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
    build_target(T "gp_client_dns_config" SRC ${client_dns_config_src} LIBS
        ${lib3_dep})
    build_target(T "gp_client_static_route" SRC ${client_static_route_src} LIBS
        ${lib3_dep})
    build_target(T "gp_client_default_gateway" SRC ${client_default_gateway_src}
        LIBS ${lib3_dep})
    build_target(T "gp_client_manage_service" SRC ${client_manage_service_src}
        LIBS ${lib3_dep})
    build_target(T "gp_client_interface" SRC ${client_interface_src} LIBS ${lib3_dep})
    build_target(T "gp_client_match_option" SRC ${client_match_option_src} LIBS
        ${lib3_dep})
    build_target(T "gp_client_define_rule" SRC ${client_define_rule_src} LIBS
        ${lib3_dep})
    build_target(T "gp_client_ssl_certificate" SRC ${client_ssl_certificate_src}
        LIBS ${lib3_dep})
    build_target(T "gp_client_bizserver" SRC ${client_bizserver_src} LIBS ${lib3_dep})
    build_target(T "gp_client_attack_detect" SRC ${client_attack_detect_src}
        LIBS ${lib3_dep})
    build_target(T "gp_client_site_security" SRC ${client_site_security_src}
        LIBS ${lib3_dep})
    build_target(T "gp_client_predef_sign_rule" SRC
        ${client_predef_sign_rule_src} LIBS ${lib3_dep})
    build_target(T "gp_client_virtual_service" SRC ${client_virtual_service_src}
        LIBS ${lib3_dep})


	#add_custom_command(TARGET gp_thread
	#		           POST_BUILD
	#			       COMMAND make test
	#)

endif()
