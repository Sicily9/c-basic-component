#include "gp.h"

void on_message(gp_tcp_connection *conn, gp_buffer *buffer)
{
	char *data = retrieve_all_as_string(buffer);
	printf("receive msg :%s\n", data);
	free(data);

	char *msg = "fuck you";
	conn_send(conn, msg, strlen(msg));
}

void on_connection(gp_tcp_connection *conn)
{
}

void timer_func4(void *data)
{
    printf("i am 4\n");
}


int main()
{
	printf("EPOLL_CTL_ADD:1 EPOLL_CTL_MOD:3 EPOLL_CTL_DEL:2\n");
	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_inet_address *address = NULL;
	create_gp_inet_address(&address, "127.0.0.1", 8000, 0);

	gp_tcp_server *server = get_tcp_server();
	init_gp_tcp_server(server, loop, address, "guish");
	
	server_set_message_callback(server, on_message);
	server_set_connection_callback(server, on_connection);

	start_server(server);

	//gp_loop_timer_start(loop, timer_func4, NULL, 2000, 1);
	
	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
