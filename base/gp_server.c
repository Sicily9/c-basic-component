#include "gp.h"
#include <sys/socket.h>

static void queue_in_loop_destroy_conn(gp_pending_task *task)
{
	connection_destroyed(task->conn);
}

static void run_in_loop_remove_conn(gp_pending_task *task)
{
	printf("run in loop remove_conn, fd:%d\n", task->conn->fd);
	gp_pending_task *new_task = NULL;
	gp_server *server = task->conn->server;
	create_gp_pending_task(&new_task, GP_RUN_IN_LOOP_CONN, queue_in_loop_destroy_conn, NULL, task->conn, NULL, 0);
	gp_queue_in_loop(server->loop, new_task);
}

void remove_conn(gp_connection *conn)
{
	dictDelete(conn->server->connections, &conn->handler.fd);
	conn_ref_dec(&conn);

	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_REMOVE_CONN, run_in_loop_remove_conn, NULL, conn, NULL, 0);
	gp_run_in_loop(conn->server->loop, task);

}

void run_in_loop_create_conn(gp_pending_task *task)
{
	char local[40] = {0};
    get_gp_sock_address(&task->conn->local_addr, local, 40);
    char peer[40] = {0};
    get_gp_sock_address(&task->conn->peer_addr, peer, 40);
    printf("run in loop create_conn, fd:%d, connection:%s->%s\n", task->conn->fd, peer, local);

	connection_established(task->conn);
}

gp_server* get_server_from_acceptor(gp_acceptor *acceptor){
        return container_of(acceptor, gp_server, acceptor);
}

void new_connection_callback(gp_acceptor *acceptor, int32_t sockfd, struct sockaddr *peer_addr)
{
	gp_server * server = get_server_from_acceptor(acceptor);
	gp_loop *loop = server->loop;

	struct sockaddr *local_addr = create_sockaddr(peer_addr);

    int len = get_gp_sock_len_by_sockaddr(peer_addr);
    getsockname(sockfd, local_addr, (socklen_t *)&len);

	gp_connection *conn = NULL;
	create_gp_connection(&conn, server, loop, sockfd, local_addr, peer_addr);
	dictAdd(server->connections, &sockfd, conn); 
	conn_ref_inc(&conn);

	conn_set_connection_callback(conn, server->connection_callback); 
	conn_set_message_callback(conn, server->message_callback);
	conn_set_write_complete_callback(conn, server->write_complete_callback);
	conn_set_close_callback(conn, remove_conn);

	gp_pending_task *task = NULL;
	create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, run_in_loop_create_conn, NULL, conn, NULL, 0);
	gp_run_in_loop(loop, task);

	conn_ref_dec(&conn);
}

void run_in_loop_server_start(gp_pending_task *task)
{
	gp_acceptor *acceptor = &task->server->acceptor;
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

	init_gp_acceptor(&server->acceptor, server->loop, listen_addr);
	set_new_connection_callback(&server->acceptor, new_connection_callback);
}

void create_gp_server(gp_server **server, gp_loop *loop, gp_sock_address *listen_addr, char *name)
{
	gp_server *tmp = malloc(sizeof(gp_server));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_server(tmp, loop, listen_addr, name);
	*server = tmp;
}


