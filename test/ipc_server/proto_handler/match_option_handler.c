#include "ipc_server/ipc_server.h"
#include "ipc_server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t match_option_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	MatchOption *match_option = (MatchOption *)msg;

	printf("match_option:{field:%s, match_rule:%d, expression:%s}\n", match_option->field, match_option->match_rule, match_option->expression);

	char *str = "i receive the match_option msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_match_option_handler(void)
{
	register_msg_callback("MatchOption", match_option_handler);
	register_name_pb_map("MatchOption", &match_option__descriptor);
}

gp_module_init(match_option_handler, MODULE_INIT_FIRST, "match_option_handler", NULL, early_init_match_option_handler);
