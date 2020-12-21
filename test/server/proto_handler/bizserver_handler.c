#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t bizserver_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	Bizserver *bizserver = (Bizserver *)msg;

	printf("bizserver:{protocol:%s, address:%s, port:%s}\n", bizserver->protocol, bizserver->address, bizserver->port);

	char *str = "i receive the bizserver msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);

	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_bizserver_handler(void)
{
	register_msg_callback("Bizserver", bizserver_handler);
	register_name_pb_map("Bizserver", &bizserver__descriptor);
}

gp_module_init(bizserver_handler, MODULE_INIT_FIRST, "bizserver_handler", NULL, early_init_bizserver_handler);
