#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t manage_service_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	ManageService *manage_service = (ManageService *)msg;

	printf("manage_service:{id:%d, name:%s, port:%s, whitelist:%s, ser_interface:%s}\n", manage_service->id, manage_service->name, manage_service->port, manage_service->whitelist, manage_service->ser_interface);

	char *str = "i receive the manage_service msg";
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_manage_service_handler(void)
{
	register_msg_callback("ManageService", manage_service_handler);
	register_name_pb_map("ManageService", &manage_service__descriptor);
}

gp_module_init(manage_service_handler, MODULE_INIT_FIRST, "manage_service_handler", NULL, early_init_manage_service_handler);
