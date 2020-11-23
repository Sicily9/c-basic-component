#include "gp.h"

void init_gp_pending_task(gp_pending_task *task, int8_t type, gp_pending_func func, gp_tcp_server *server, gp_tcp_connection *conn, char *msg, int len)
{
	task->type = type;
	task->tcp_server = server;
	task->conn = conn;
	task->msg = malloc(len);
	memcpy(task->msg, msg ,len);
	task->len = len;

	task->pending_func = func;
	GP_LIST_NODE_INIT(&task->pending_task_node);
}


void create_gp_pending_task(gp_pending_task **task, int8_t type, gp_pending_func func, gp_tcp_server *server, gp_tcp_connection *conn, char *msg, int len)
{
	gp_pending_task *tmp = malloc(sizeof(gp_pending_task));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_pending_task(tmp, type, func, server, conn, msg, len);
	*task = tmp;
}

void destruct_gp_pending_task(gp_pending_task *task)
{
	gp_list_node_remove(&task->pending_task_node);
	if(task->msg){
		free(task->msg);
	}
	free(task);
}

