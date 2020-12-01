#include "gp.h"
#include <sys/socket.h>

void connection_handler_close(gp_handler *handler)
{
	gp_server *server = get_server();
	gp_connection *conn = dictFetchValue(server->connections, &handler->fd);
	conn_ref_inc(&conn);

	conn->state = k_disconnected;
	disable_all(handler);
	
	conn->connection_callback(conn);
	conn->close_callback(conn);
	conn_ref_dec(&conn);
}

void connection_handler_error(gp_handler *handler)
{

}

void connection_handler_write(gp_handler *handler)
{
	gp_server *server = get_server();
	gp_connection *conn = dictFetchValue(server->connections, &handler->fd);

	size_t n = write(handler->fd, peek(conn->output_buffer), readable_bytes(conn->output_buffer));
	if (n > 0)
	{
		retrieve(conn->output_buffer, n);
		if(readable_bytes(conn->output_buffer) == 0)
		{
			disable_writing(handler);
			if(conn->write_complete_callback)
			{
				//
			}
			if(conn->state == k_disconnecting)
			{
				shutdown(handler->fd, SHUT_WR);
			}
		}
	}
}

void connection_handler_read(gp_handler *handler)
{
	gp_server *server = get_server();
	gp_connection *conn = dictFetchValue(server->connections, &handler->fd);
	
	int saved_errno = 0;
	printf("buffer len:%ld\n", readable_bytes(conn->input_buffer));
	size_t n = buffer_read_fd(conn->input_buffer, handler->fd, &saved_errno);
	printf("after receive %ld bytes, buffer len:%ld\n", n, readable_bytes(conn->input_buffer));
	if(n > 0)
	{
		conn_ref_inc(&conn);
		conn->message_callback(conn, conn->input_buffer);
		conn_ref_dec(&conn);
	}else if (n == 0) {
		connection_handler_close(handler); //对方直接close时
	}else {
		errno = saved_errno;
		connection_handler_error(handler);
	}

}

void connection_established(gp_connection *conn)
{
	conn->state = k_connected;
	enable_reading(conn->handler);

	conn->connection_callback(conn);
}

void connection_destroyed(gp_connection *conn)
{
	if(conn->state == k_connected){
		conn->state = k_disconnected;
		disable_all(conn->handler);
		conn->connection_callback(conn);
	}
	handler_remove(conn->handler);
}

void conn_send_in_loop(gp_connection *conn, char *msg, int len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    int8_t faultError = 0;
    if (conn->state == k_disconnected)
        return;
    // if no thing in output queue, try writing directly
    if (!is_writing(conn->handler) && readable_bytes(conn->output_buffer) == 0)
    {
        nwrote = write(conn->handler->fd, msg, len);
		printf("nwrote:%ld, %d send_msg\n", nwrote, conn->handler->fd);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && conn->write_complete_callback)
            {
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = 1;
                }
            }
        }
    }

    if (!faultError && remaining > 0)
    {
        //size_t oldLen = readable_bytes(conn->output_buffer);
		//TODO high water mark
        buffer_append(conn->output_buffer, msg + nwrote, remaining);
        if (is_writing(conn->handler))
        {
            enable_writing(conn->handler);
        }
    }
}

void run_in_loop_trans(gp_server *server, gp_connection *conn, char *msg, int len)
{
	conn_send_in_loop(conn, msg, len);
}

void conn_send(gp_connection *conn, char *data, int len)
{
	if(conn->state == k_connected)
	{
		if(is_in_loop_thread(conn->loop))
		{
			conn_send_in_loop(conn, data, len);
		}else{
			gp_pending_task *task = NULL;
			create_gp_pending_task(&task, GP_RUN_IN_LOOP_TRANS, run_in_loop_trans, NULL, conn, data, len);
			gp_run_in_loop(conn->loop, task);
		}
	}
}

void run_in_loop_shutdown(gp_server *server, gp_connection *conn, char *msg, int len)
{
	if(!is_writing(conn->handler))
	{
		shutdown(conn->fd, SHUT_WR);
	}
}

void conn_shutdown(gp_connection *conn)
{
	if(conn->state == k_connected)
	{
		conn->state = k_disconnecting;
	
		gp_pending_task *task = NULL;
		create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, run_in_loop_shutdown, NULL, conn, NULL, 0);	
		gp_run_in_loop(conn->loop, task);
	}
}

void queue_in_loop_force_close(gp_server *server, gp_connection *conn, char *msg, int len)
{
	if(conn->state == k_connected || conn->state == k_disconnecting)
	{	
		connection_handler_close(conn->handler);
	}
}

void conn_force_close(gp_connection *conn)
{
	if(conn->state == k_connected || conn->state == k_disconnecting)
	{
		conn->state = k_disconnecting;	
		gp_pending_task *task = NULL;
		create_gp_pending_task(&task, GP_RUN_IN_LOOP_CONN, queue_in_loop_force_close, NULL, conn, NULL, 0);	
		gp_queue_in_loop(conn->loop, task);
	}
}

void destruct_gp_connection(gp_connection *conn)
{
	destruct_gp_handler(conn->handler);
	destruct_gp_buffer(conn->input_buffer);
	destruct_gp_buffer(conn->output_buffer);
	free(conn);
}

void init_gp_connection(gp_connection *conn, gp_loop *loop, int32_t fd, struct sockaddr *localaddr, struct sockaddr *peeraddr)
{
	conn->loop = loop;
	conn->fd = fd;
	conn->state = k_connecting;
	conn->message_callback = NULL;
	conn->connection_callback = NULL;
	conn->write_complete_callback = NULL;
	gp_atomic_set(&conn->ref, 1);

    init_gp_sock_address_by_sockaddr(&conn->local_addr, localaddr);
    init_gp_sock_address_by_sockaddr(&conn->peer_addr, peeraddr);

    char local[40] = {0};
    get_gp_sock_address(&conn->local_addr, local);
    char peer[40] = {0};
    get_gp_sock_address(&conn->peer_addr, peer);
    printf("fd:%d, connection:%s->%s\n", conn->fd, peer, local);

	create_gp_handler(&conn->handler, loop, fd);
	create_gp_buffer(&conn->input_buffer);
	create_gp_buffer(&conn->output_buffer);

	set_read_callback(conn->handler, connection_handler_read);
	set_write_callback(conn->handler, connection_handler_write);
	set_close_callback(conn->handler, connection_handler_close);
	set_error_callback(conn->handler, connection_handler_error);
}

void create_gp_connection(gp_connection **connection, gp_loop *loop, int32_t fd, struct sockaddr *localaddr, struct sockaddr *peeraddr)
{
	gp_connection *tmp = malloc(sizeof(gp_connection));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_connection(tmp, loop, fd, localaddr, peeraddr);
	*connection = tmp;
}

void conn_ref_inc(gp_connection **conn)
{
	gp_atomic_inc(&(*conn)->ref);
	//printf("conn ref: %ld\n", gp_atomic_get(&(*conn)->ref));
}

void conn_ref_dec(gp_connection **conn)
{
	if(gp_atomic_dec_and_test(&(*conn)->ref)){
		destruct_gp_connection(*conn);
		*conn = NULL;
	}
	//	printf("conn ref: %ld\n", (*conn != NULL) ? gp_atomic_get(&(*conn)->ref) : 0);
}

void conn_set_write_complete_callback(gp_connection * conn, gp_write_complete_callback write_complete_callback)
{
	conn->write_complete_callback = write_complete_callback;
}

void conn_set_message_callback(gp_connection * conn, gp_message_callback callback)
{
	conn->message_callback = callback;
}

void conn_set_connection_callback(gp_connection * conn, gp_connection_callback callback)
{
	conn->connection_callback = callback;
}

void conn_set_close_callback(gp_connection * conn, gp_close_callback callback)
{
	conn->close_callback = callback;
}
