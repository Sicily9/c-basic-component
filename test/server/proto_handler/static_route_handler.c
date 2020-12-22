#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t static_route_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	StaticRoute *static_route = (StaticRoute *)msg;

	printf("static_route:{id:%d, interface_name:%s, dst_ip:%s, subnet_mask:%s, gateway:%s}\n", static_route->id, static_route->interface_name, static_route->dst_ip, static_route->subnet_mask, static_route->gateway);

	char *str = "i receive the static_route msg";
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_static_route_handler(void)
{
	register_msg_callback("StaticRoute", static_route_handler);
	register_name_pb_map("StaticRoute", &static_route__descriptor);
}

gp_module_init(static_route_handler, MODULE_INIT_FIRST, "static_route_handler", NULL, early_init_static_route_handler);
