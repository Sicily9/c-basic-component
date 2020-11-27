#include "guish_server.h"
#include "proto_interface/test.pb-c.h"

uint32_t person_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	Person *person_msg = (Person *)msg;
	printf("person_id:%d name number:%d\n", person_msg->id, person_msg->n_name);

	for(int i = 0; i < person_msg->n_name; i++){
		printf("name%d:%s", i+1, person_msg->name[i]->name);
	}

	conn_send(conn, "fuck you", 8);
	return 0;
}

void early_init_person_proto(void)
{
	register_msg_callback("Person", person_handler);
	register_name_pb_map("Person", &person__descriptor);
}

gp_module_init(proto_module, MODULE_INIT_FIRST, "person_handler", NULL, early_init_person_proto);
