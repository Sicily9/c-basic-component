#include "guish_server.h"

/*				client-server protocol format
 *                -------------------------
 *				 |         4 bytes         | 
 *				 |     magic:0x1343EA4     |
 *				 |-------------------------|             
 *				 |	       4 bytes         |	
 *               |           len           |
 *               |-------------------------|                   
 *               |         4 bytes         |
 *               |        name_len         | 
 *               |-------------------------|
 *               |      name_len bytes     |
 *               |           name          |
 *               |-------------------------| 
 *               | len - 4 - name_len bytes|   
 *               |       protobuf-msg      |
 *                -------------------------          
 */                                        

gp_task_processor *task_processor = NULL;

uint32_t protobuf_default_callback(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	int port = 0;
	char ip[20];
	get_peer_address(conn->fd, ip, &port, 20);
	printf("%s:%d send protobuf_msg name:%s,  unknown message type\n", ip, port, msg->descriptor->name);
	return 0;
}

void handle_msg(void *msg)
{
	transport_msg *tmsg = msg;
	ProtobufCMessage *pbmsg = tmsg->msg;
	gp_tcp_connection *conn = tmsg->conn;
	conn_ref_inc(&conn);

	gp_protobuf_msg_callback cb = get_msg_callback(pbmsg->descriptor->name);
	if(likely(cb != NULL))
		cb(conn, pbmsg);
	else
		protobuf_default_callback(conn, pbmsg);

    protobuf_c_message_free_unpacked(pbmsg, NULL);
	conn_ref_dec(&conn);
}

void on_message(gp_tcp_connection *conn, gp_buffer *buffer)
{
    while (readable_bytes(buffer) >= 8)
	{   
        ProtobufCMessage *msg = decode(buffer);
		if(msg){
			transport_msg *tmsg;
			create_transport_msg(&tmsg, msg, conn);

			gp_task *task;
			create_task(&task, get_task_processor(), handle_msg, tmsg);
			run_task(task);

			destroy_task(task);
			free(tmsg);
		}
	}
}

void on_connection(gp_tcp_connection *conn)
{

}

void guish_start_server(guish_server *server)
{
	start_server(server->tcp_server);
}

void init_guish_server(guish_server *server, gp_loop *loop, gp_inet_address *address, char *name)
{
	gp_tcp_server *tcp_server = get_tcp_server();
	server->loop = loop;
	server->tcp_server = tcp_server;
	init_gp_tcp_server(tcp_server, loop, address, "guish");
	
	server_set_message_callback(tcp_server, on_message);
	server_set_connection_callback(tcp_server, on_connection);
}

void create_guish_server(guish_server **server, gp_loop *loop, gp_inet_address *address, char *name)
{
	guish_server *tmp = malloc(sizeof(guish_server));
	memset(tmp, 0, sizeof(guish_server));
	init_guish_server(tmp, loop, address, name);
	*server = tmp;
}

void init_transport_msg(transport_msg *tmsg, ProtobufCMessage *msg, gp_tcp_connection *conn)
{
	tmsg->msg = msg;
	tmsg->conn = conn;
}

void create_transport_msg(transport_msg **tmsg, ProtobufCMessage *msg, gp_tcp_connection *conn)
{
	transport_msg *tmp = malloc(sizeof(transport_msg));
	memset(tmp, 0, sizeof(transport_msg));
	init_transport_msg(tmp, msg, conn);
	*tmsg = tmp;
}

gp_task_processor *get_task_processor(void)
{
	if(task_processor == NULL){
		create_task_processor(&task_processor, 1);
	}
	return task_processor;
}
