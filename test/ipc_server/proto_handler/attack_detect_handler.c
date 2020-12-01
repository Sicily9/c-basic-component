#include "ipc_server/ipc_server.h"
#include "ipc_server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t attack_detect_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	AttackDetect *attack_detect = (AttackDetect *)msg;

	printf("attack_detect:{name:%s, enable:%d, block:%d, log:%d}\n", attack_detect->name, attack_detect->enable, attack_detect->block, attack_detect->log);

	char *str = "i receive the attack_detect msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_attack_detect_handler(void)
{
	register_msg_callback("AttackDetect", attack_detect_handler);
	register_name_pb_map("AttackDetect", &attack_detect__descriptor);
}

gp_module_init(attack_detect_handler, MODULE_INIT_FIRST, "attack_detect_handler", NULL, early_init_attack_detect_handler);
