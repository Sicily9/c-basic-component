#include "guish_server.h"
#include "proto/test.pb-c.h"

uint32_t msg_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	Name *name_msg = (Name *)msg;
	printf("name:%s id:%d age:%d\n",name_msg->name, name_msg->id, name_msg->age);
	return 0;
}

void early_init_proto(void)
{
	register_msg_callback("Name", msg_handler);
	register_name_pb_map("Name", &name__descriptor);
}

gp_module_init(proto_module, MODULE_INIT_FIRST, "name_handler", NULL, early_init_proto);
