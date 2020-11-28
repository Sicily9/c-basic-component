#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t default_gateway_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	DefaultGateway *default_gateway = (DefaultGateway *)msg;

	printf("default_gateway:{id:%d, interface_name:%s, ipv4_gateway:%s, ipv6_gateway:%s}\n", default_gateway->id, default_gateway->interface_name, default_gateway->ipv4_gateway, default_gateway->ipv6_gateway);

	char *str = "i receive the default_gateway msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_default_gateway_handler(void)
{
	register_msg_callback("DefaultGateway", default_gateway_handler);
	register_name_pb_map("DefaultGateway", &default_gateway__descriptor);
}

gp_module_init(default_gateway_handler, MODULE_INIT_FIRST, "default_gateway_handler", NULL, early_init_default_gateway_handler);
