#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t predef_sign_rule_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	PredefSignRule *predef_sign_rule = (PredefSignRule *)msg;

	printf("predef_sign_rule:{action:%d, predef_sign_rule_path:%s}\n", predef_sign_rule->action, predef_sign_rule->predef_sign_rule_path);

	char *str = "i receive the predef_sign_rule msg";
	printf("fd:%d, send msg:%s, len:%d\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_predef_sign_rule_handler(void)
{
	register_msg_callback("PredefSignRule", predef_sign_rule_handler);
	register_name_pb_map("PredefSignRule", &predef_sign_rule__descriptor);
}

gp_module_init(predef_sign_rule_handler, MODULE_INIT_FIRST, "predef_sign_rule_handler", NULL, early_init_predef_sign_rule_handler);
