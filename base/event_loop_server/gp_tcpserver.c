#include "gp.h"
#include <sys/socket.h>

gp_tcp_server *tcp_server = NULL;

gp_tcp_server * get_tcp_server(void){
	if(unlikely(tcp_server == NULL)){
		tcp_server = malloc(sizeof(gp_tcp_server));
	}
	return tcp_server;
}


static void queue_in_loop_destroy_conn(gp_tcp_server * server, gp_tcp_connection *conn, char *msg, int len)
{
	connection_destroyed(conn);
}

static void run_in_loop_remove_conn(gp_tcp_server *server, gp_tcp_connection *conn, char *msg, int len)
{
	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, queue_in_loop_destroy_conn, NULL, conn, NULL, 0);
	gp_queue_in_loop(server->loop, task);
}


void remove_conn(gp_tcp_connection *conn)
{
	gp_tcp_server *server = get_tcp_server();
	dictDelete(server->connections, &conn->handler->fd);
	conn_ref_dec(&conn);
	printf("remove conn from connections fd:%d\n", conn->handler->fd);

	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_REMOVE_CONN, run_in_loop_remove_conn, server, conn, NULL, 0);
	gp_run_in_loop(server->loop, task);

}

void run_in_loop_create_conn(gp_tcp_server * server, gp_tcp_connection *conn, char *msg, int len)
{
	connection_established(conn);
}

void new_connection_callback(int32_t sockfd, gp_inet_address *peeraddr)
{
	gp_tcp_server * tcp_server = get_tcp_server();
	gp_loop *loop = tcp_server->loop;

	gp_inet_address local_addr;
	struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = (socklen_t)(sizeof localaddr);
    getsockname(sockfd, (struct sockaddr *)(&localaddr), &addrlen);

	local_addr.addr = localaddr;

	gp_tcp_connection *conn = NULL;
	create_gp_tcp_connection(&conn, loop, sockfd, &local_addr, peeraddr);
	dictAdd(tcp_server->connections, &sockfd, conn); 
	conn_ref_inc(&conn);

	conn_set_connection_callback(conn, tcp_server->connection_callback); 
	conn_set_message_callback(conn, tcp_server->message_callback);
	conn_set_write_complete_callback(conn, tcp_server->write_complete_callback);
	conn_set_close_callback(conn, remove_conn);

	gp_pending_task *task = NULL;
	printf("create conn pending_task\n");
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, run_in_loop_create_conn, NULL, conn, NULL, 0);
	gp_run_in_loop(loop, task);

	conn_ref_dec(&conn);
}

void run_in_loop_server_start(gp_tcp_server * server, gp_tcp_connection *conn, char *msg, int len)
{
	gp_acceptor *acceptor = server->acceptor;
	acceptor_listen(acceptor);
}

void start_server(gp_tcp_server *server)
{
	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_SERVER_START, run_in_loop_server_start, server, NULL, NULL, 0);
	gp_run_in_loop(server->loop, task);
}

void server_set_write_complete_callback(gp_tcp_server * conn, gp_write_complete_callback write_complete_callback)
{
    conn->write_complete_callback = write_complete_callback;
}

void server_set_message_callback(gp_tcp_server * conn, gp_message_callback callback)
{
    conn->message_callback = callback;
}

void server_set_connection_callback(gp_tcp_server * conn, gp_connection_callback callback)
{
    conn->connection_callback = callback;
}                                             

void init_gp_tcp_server(gp_tcp_server *tcp_server, gp_loop *loop, gp_inet_address *listen_addr, char *name)
{
	tcp_server->loop = loop;
	strcpy(tcp_server->name, name);
	tcp_server->next_conn_id = 1;
	tcp_server->message_callback = NULL;
	tcp_server->connection_callback = NULL;
	tcp_server->write_complete_callback = NULL;
	tcp_server->connections = dictCreate(INT_DICT, 0);

	create_gp_acceptor(&tcp_server->acceptor, tcp_server->loop, listen_addr);
	set_new_connection_callback(tcp_server->acceptor, new_connection_callback);
}

void create_gp_tcp_server(gp_tcp_server **tcp_server, gp_loop *loop, gp_inet_address *listen_addr, char *name)
{
	gp_tcp_server *tmp = malloc(sizeof(gp_tcp_server));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_tcp_server(tmp, loop, listen_addr, name);
	*tcp_server = tmp;
}


