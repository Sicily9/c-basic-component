#include "gp.h"

static void acceptor_read_callback(gp_handler *handler)
{
	gp_acceptor * acceptor = get_server()->acceptor;

	struct sockaddr addr;
    bzero(&addr, sizeof addr);
	socklen_t addrlen = (socklen_t)(sizeof addr);
    int32_t connfd = accept4(handler->fd, (struct sockaddr *)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

	{
        #if 0
		char serv_ip[20], cli_ip[20];
		int port, cli_port;
		get_local_address(connfd, serv_ip, &port, 20);
		get_peer_address(connfd, cli_ip, &cli_port, 20);
		printf("fd:%d, new connection: %s:%d -> %s:%d\n", connfd, cli_ip, cli_port, serv_ip, port);
        #endif
	}

	if(connfd > 0){
		acceptor->new_connection_callback(connfd, &addr);
	}else{
		close(handler->fd);	
	}

}

void set_new_connection_callback(gp_acceptor *acceptor, gp_new_connection_callback new_connection_callback)
{
	acceptor->new_connection_callback = new_connection_callback;
}

void init_gp_acceptor(gp_acceptor *acceptor, gp_loop *loop, gp_sock_address *sock_address)
{
	acceptor->loop = loop;
	acceptor->listenning = 0;
	acceptor->fd = socket(((struct sockaddr *)sock_address)->sa_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0); 
	printf("listen fd:%d\n", acceptor->fd);
	create_gp_handler(&acceptor->accept_handler, loop, acceptor->fd);

	int32_t optval = 1;
	setsockopt(acceptor->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)(sizeof optval));

	bind(acceptor->fd, (struct sockaddr*)&(sock_address->addr), sizeof(sock_address->addr));

	set_read_callback(acceptor->accept_handler, acceptor_read_callback);

}

void create_gp_acceptor(gp_acceptor **acceptor, gp_loop *loop, gp_sock_address *sock_address)
{
	gp_acceptor *tmp = malloc(sizeof(gp_acceptor));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_acceptor(tmp, loop, sock_address);
	*acceptor = tmp;
}

void acceptor_listen(gp_acceptor *acceptor)
{
	acceptor->listenning = 1;
	int32_t ret = listen(acceptor->fd, SOMAXCONN);

	char serv_ip[20];
	int port;
	get_local_address(acceptor->fd, serv_ip, &port, 20);
	printf("listen address:%s:%d\n", serv_ip, port);

	if(ret < 0){
		exit(-1);
	}
	enable_reading(acceptor->accept_handler);
}