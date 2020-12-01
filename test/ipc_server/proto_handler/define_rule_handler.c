#include "ipc_server/ipc_server.h"
#include "ipc_server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t define_rule_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	DefineRule *define_rule = (DefineRule *)msg;

	printf("define_rule:{id:%d, seq:%d, mode:%d, enable:%d, level:%d, log_type:%d, name:%s, n_match_option_list:%ld expire_time:%s}\n", define_rule->id, define_rule->seq, define_rule->mode, define_rule->enable, define_rule->level, define_rule->log_type, define_rule->name, define_rule->n_match_option_list, define_rule->expire_time);

	for(int i = 0; i < define_rule->n_match_option_list; i++){
		MatchOption *match_option = define_rule->match_option_list[i];
		printf("match_option:{field:%s, match_rule:%d, expression:%s}\n", match_option->field, match_option->match_rule, match_option->expression);
	}

	char *str ="i receive the define_rule msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_define_rule_handler(void)
{
	register_msg_callback("DefineRule", define_rule_handler);
	register_name_pb_map("DefineRule", &define_rule__descriptor);
}

gp_module_init(define_rule_handler, MODULE_INIT_FIRST, "define_rule_handler", NULL, early_init_define_rule_handler);
