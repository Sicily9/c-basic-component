#include "gp.h"
#include <sys/socket.h>

gp_server *server = NULL;

gp_server * get_server(void){
	if(unlikely(server == NULL)){
		server = malloc(sizeof(gp_server));
	}
	return server;
}


static void queue_in_loop_destroy_conn(gp_server * server, gp_connection *conn, char *msg, int len)
{
	connection_destroyed(conn);
}

static void run_in_loop_remove_conn(gp_server *server, gp_connection *conn, char *msg, int len)
{
	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, queue_in_loop_destroy_conn, NULL, conn, NULL, 0);
	gp_queue_in_loop(server->loop, task);
}

void remove_conn(gp_connection *conn)
{
	gp_server *server = get_server();
	dictDelete(server->connections, &conn->handler->fd);
	conn_ref_dec(&conn);
	printf("remove conn from connections fd:%d\n", conn->handler->fd);

	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_REMOVE_CONN, run_in_loop_remove_conn, server, conn, NULL, 0);
	gp_run_in_loop(server->loop, task);

}

void run_in_loop_create_conn(gp_server * server, gp_connection *conn, char *msg, int len)
{
	connection_established(conn);
}

void new_connection_callback(int32_t sockfd, struct sockaddr *peer_addr)
{
	gp_server * server = get_server();
	gp_loop *loop = server->loop;

	struct sockaddr local_addr;
    bzero(&local_addr, sizeof local_addr);

    int len = get_gp_sock_len_by_sockaddr(peer_addr);
    getsockname(sockfd, &local_addr, (socklen_t *)&len);

	gp_connection *conn = NULL;
	create_gp_connection(&conn, loop, sockfd, &local_addr, peer_addr);
	dictAdd(server->connections, &sockfd, conn); 
	conn_ref_inc(&conn);

	conn_set_connection_callback(conn, server->connection_callback); 
	conn_set_message_callback(conn, server->message_callback);
	conn_set_write_complete_callback(conn, server->write_complete_callback);
	conn_set_close_callback(conn, remove_conn);

	gp_pending_task *task = NULL;
	printf("create conn pending_task\n");
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, run_in_loop_create_conn, NULL, conn, NULL, 0);
	gp_run_in_loop(loop, task);

	conn_ref_dec(&conn);
}

void run_in_loop_server_start(gp_server * server, gp_connection *conn, char *msg, int len)
{
	gp_acceptor *acceptor = server->acceptor;
	acceptor_listen(acceptor);
}

void start_server(gp_server *server)
{
	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_SERVER_START, run_in_loop_server_start, server, NULL, NULL, 0);
	gp_run_in_loop(server->loop, task);
}

void server_set_write_complete_callback(gp_server * server, gp_write_complete_callback write_complete_callback)
{
    server->write_complete_callback = write_complete_callback;
}

void server_set_message_callback(gp_server * server, gp_message_callback callback)
{
    server->message_callback = callback;
}

void server_set_connection_callback(gp_server * server, gp_connection_callback callback)
{
    server->connection_callback = callback;
}                                             

void init_gp_server(gp_server *server, gp_loop *loop, gp_sock_address *listen_addr, char *name)
{
	server->loop = loop;
	strcpy(server->name, name);
	server->next_conn_id = 1;
	server->message_callback = NULL;
	server->connection_callback = NULL;
	server->write_complete_callback = NULL;
	server->connections = dictCreate(INT_DICT, 0);

	create_gp_acceptor(&server->acceptor, server->loop, listen_addr);
	set_new_connection_callback(server->acceptor, new_connection_callback);
}

void create_gp_server(gp_server **server, gp_loop *loop, gp_sock_address *listen_addr, char *name)
{
	gp_server *tmp = malloc(sizeof(gp_server));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_server(tmp, loop, listen_addr, name);
	*server = tmp;
}


