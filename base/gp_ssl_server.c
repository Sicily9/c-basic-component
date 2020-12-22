#include "gp.h"

void gp_set_ssl_cert_key(gp_ssl_server *ssl_server, char * cert, char *key)
{

    /* 载入用户私钥 */
    if (SSL_CTX_use_PrivateKey_file(ssl_server->ctx, key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

/* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    if (SSL_CTX_use_certificate_file(ssl_server->ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(ssl_server->ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

}

uint32_t ssl_protobuf_default_callback(gp_connection *conn, ProtobufCMessage *msg)
{
    int port = 0;
    char ip[20];
    get_peer_address(conn->fd, ip, &port, 20);
    printf("%s:%d send protobuf_msg name:%s,  unknown message type\n", ip, port, msg->descriptor->name);
    return 0;
}

void handle_ssl_msg(gp_connection *conn, gp_buffer *buffer)
{
    if (readable_bytes(buffer) >= 8)
    {   
        ProtobufCMessage *msg = decode(buffer);
        if(msg){
            conn_ref_inc(&conn);
            gp_protobuf_msg_callback cb = get_msg_callback(msg->descriptor->name);
            if(likely(cb != NULL))
                cb(conn, msg);
            else
                ssl_protobuf_default_callback(conn, msg);

            protobuf_c_message_free_unpacked(msg, NULL);
            conn_ref_dec(&conn);
        }
    }
}

gp_ssl_server* get_ssl_server_from_server(gp_server *server){
        return container_of(server, gp_ssl_server, server);
}

void ssl_handler_handshake(gp_handler *handler)
{
	gp_connection *conn = get_connection_from_handler(handler);
    gp_ssl_server *ssl_server = get_ssl_server_from_server(conn->server);
    int r = SSL_do_handshake(conn->conn_pri);
    if (r == 1) {
        printf("SSL_connected success\n");
        set_read_callback(&conn->handler, ssl_handler_read);
        server_set_message_callback(&ssl_server->server, handle_ssl_msg);
        conn_set_message_callback(conn, handle_ssl_msg);
        return;
    }

    int err = SSL_get_error(conn->conn_pri, r);
    if (err == SSL_ERROR_WANT_WRITE) {
        printf("want write!!!\n");
        enable_writing(&conn->handler);
        disable_reading(&conn->handler);
    } else if (err == SSL_ERROR_WANT_READ) {
        printf("want read!!!\n");
        enable_reading(&conn->handler);
        disable_writing(&conn->handler);
    } else {
        printf("ssl_handler_handshake err!!!\n");
        ERR_print_errors_fp(stderr);
        //connection_handler_close(handler);
    }
}

void ssl_handler_write(gp_handler *handler)
{
	gp_connection *conn = get_connection_from_handler(handler);

	size_t n = SSL_write(conn->conn_pri, peek(conn->output_buffer), readable_bytes(conn->output_buffer));
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

void ssl_handler_close(gp_handler *handler)
{
	gp_connection *conn = get_connection_from_handler(handler);
	conn_ref_inc(&conn);

    conn->close_callback(conn);
    
    SSL_free(conn->conn_pri);
	conn_ref_dec(&conn);
}

void ssl_handler_read(gp_handler *handler)
{
    gp_connection *conn = get_connection_from_handler(handler);

    int writable = writable_bytes(conn->input_buffer);
    printf("writable:%d\n", writable);
    char *buf = conn->input_buffer->buffer + conn->input_buffer->writer_index;
    int n = SSL_read(conn->conn_pri, buf, writable);

    int err = SSL_get_error(conn->conn_pri, n);
    if (n < 0) {
        char szErrMsg[1024] = {0};
        ERR_error_string(err,szErrMsg);
        printf("n:%d errno:%d errmsg:%s, err:%d msg:%s\n", n, errno, strerror(errno), err, szErrMsg);
        if(errno == ECONNRESET)
            ssl_handler_close(handler);
    } else if (n > 0 ){
        if( n < writable){
            conn->input_buffer->writer_index += n;
        } else {
            conn->input_buffer->writer_index = conn->input_buffer->len;
            int len = (int)(0.5 * conn->input_buffer->len);
            printf("add len :%d\n", len);
            ensure_writable_bytes(conn->input_buffer, len);
            printf("after add len :%ld\n", conn->input_buffer->len);
        }
		conn_ref_inc(&conn);
		conn->message_callback(conn, conn->input_buffer);
		conn_ref_dec(&conn);
    } else {
        char szErrMsg[1024] = {0};
        ERR_error_string(err,szErrMsg);
        printf("n:%d errno:%d errmsg:%s, err:%d msg:%s\n", n, errno, strerror(errno), err, szErrMsg);
        return ;
    }
}

void ssl_connection_init(gp_connection *conn)
{
    if(conn->state == k_connected){
        printf("ssl connection init\n");
        gp_ssl_server *ssl_server = get_ssl_server_from_server(conn->server);
        conn->conn_pri = SSL_new(ssl_server->ctx);
        SSL_set_fd(conn->conn_pri, conn->fd);
        SSL_set_accept_state(conn->conn_pri);

        set_close_callback(&conn->handler, ssl_handler_close);
        set_read_callback(&conn->handler, ssl_handler_handshake);
        set_write_callback(&conn->handler, ssl_handler_handshake);
        printf("ssl connection init finish\n");
    } else if(conn->state == k_disconnected) {
        printf("ssl connection over\n");
    }
}

void gp_ssl_start_server(gp_ssl_server *server)
{
	start_server(&server->server);
}

void init_gp_ssl_server(gp_ssl_server *ssl_server, gp_loop *loop, gp_sock_address *address, char *name)
{
    init_gp_server(&ssl_server->server, loop, address, "gp_ssl");
	ssl_server->loop = loop;

    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ssl_server->ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ssl_server->ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

	server_set_connection_callback(&ssl_server->server, ssl_connection_init);
}

void create_gp_ssl_server(gp_ssl_server **server, gp_loop *loop, gp_sock_address *address, char *name)
{
	gp_ssl_server *tmp = malloc(sizeof(gp_ssl_server));
	memset(tmp, 0, sizeof(gp_ssl_server));
	init_gp_ssl_server(tmp, loop, address, name);
	*server = tmp;
}
