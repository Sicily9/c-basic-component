#include "gp.h"

uint32_t protobuf_default_callback(gp_connection *conn, ProtobufCMessage *msg)
{
    int port = 0;
    char ip[20];
    get_peer_address(conn->fd, ip, &port, 20);
    printf("%s:%d send protobuf_msg name:%s,  unknown message type\n", ip, port, msg->descriptor->name);
    return 0;
}

void on_message(gp_connection *conn, gp_buffer *buffer)
{
    while (readable_bytes(buffer) >= 8)
    {   
        ProtobufCMessage *msg = decode(buffer);
        if(msg){
            conn_ref_inc(&conn);
            gp_protobuf_msg_callback cb = get_msg_callback(msg->descriptor->name);
            if(likely(cb != NULL))
                cb(conn, msg);
            else
                protobuf_default_callback(conn, msg);

            protobuf_c_message_free_unpacked(msg, NULL);
            conn_ref_dec(&conn);
        }
    }
}

void on_connection(gp_connection *conn)
{

}

void gp_ipc_start_server(gp_ipc_server *server)
{
	start_server(server->server);
}

void init_gp_ipc_server(gp_ipc_server *tcp_server, gp_loop *loop, gp_sock_address *address, char *name)
{
	gp_server *server = get_server();
	tcp_server->loop = loop;
	tcp_server->server = server;
	init_gp_server(server, loop, address, "gp_ipc");
	
	server_set_message_callback(server, on_message);
	server_set_connection_callback(server, on_connection);
}

void create_gp_ipc_server(gp_ipc_server **server, gp_loop *loop, gp_sock_address *address, char *name)
{
	gp_ipc_server *tmp = malloc(sizeof(gp_ipc_server));
	memset(tmp, 0, sizeof(gp_ipc_server));
	init_gp_ipc_server(tmp, loop, address, name);
	*server = tmp;
}

void gp_ipc_start(void)
{
    gp_loop *loop = NULL;
    create_gp_loop(&loop);

    char *process_name = NULL;
    get_process_name(&process_name);

    char path[40] = {0};
    snprintf(path, 40, "/tmp/%s.socket", process_name);

    gp_sock_address *address = NULL;
    create_gp_sock_address(&address, path, -1, DOMAIN);
    free(process_name);

    gp_ipc_server *server = NULL;
    create_gp_ipc_server(&server, loop, address, "ipc");

    gp_ipc_start_server(server);

    gp_loop_run(loop, GP_RUN_DEFAULT);
}
