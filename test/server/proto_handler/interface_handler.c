#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t interface_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	Interface *interface = (Interface *)msg;

	printf("interface:{id:%d, name:%s, type:%d, speed:%d, ipv4:%s, ipv6:%s, mac:%s, mtu:%s, negotiate:%d, half:%d}\n", interface->id, interface->name, interface->type, interface->speed, interface->ipv4, interface->ipv6, interface->mac, interface->mtu, interface->negotiate, interface->half);

	char *str = "i receive the interface msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_interface_handler(void)
{
	register_msg_callback("Interface", interface_handler);
	register_name_pb_map("Interface", &interface__descriptor);
}

gp_module_init(interface_handler, MODULE_INIT_FIRST, "interface_handler", NULL, early_init_interface_handler);
