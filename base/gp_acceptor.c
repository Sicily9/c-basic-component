#include "gp.h"

static void acceptor_read_callback(gp_handler *handler)
{
	gp_acceptor * acceptor = get_tcp_server()->acceptor;

	struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
	socklen_t addrlen = (socklen_t)(sizeof addr);
    int32_t connfd = accept4(handler->fd, (struct sockaddr *)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

	gp_inet_address peeraddr;
	peeraddr.addr = addr;

	if(connfd > 0){
		acceptor->new_connection_callback(handler->fd, &peeraddr);
	}else{
		close(handler->fd);	
	}

}

void set_new_connection_callback(gp_acceptor *acceptor, gp_new_connection_callback new_connection_callback)
{
	acceptor->new_connection_callback = new_connection_callback;
}

void init_gp_acceptor(gp_acceptor *acceptor, gp_loop *loop, gp_inet_address *inet_address)
{
	acceptor->loop = loop;
	acceptor->listenning = 0;
	acceptor->fd = socket(inet_address->addr.sin_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP); 
	create_gp_handler(&acceptor->accept_handler, loop, acceptor->fd);

	int32_t optval = 1;
	setsockopt(acceptor->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)(sizeof optval));

	bind(acceptor->fd, (struct sockaddr*)&(inet_address->addr), sizeof(inet_address->addr));

	set_read_callback(acceptor->accept_handler, acceptor_read_callback);

}

void create_gp_acceptor(gp_acceptor **acceptor, gp_loop *loop, gp_inet_address *inet_address)
{
	gp_acceptor *tmp = malloc(sizeof(gp_acceptor));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_acceptor(tmp, loop, inet_address);
	*acceptor = tmp;
}

void acceptor_listen(gp_acceptor *acceptor)
{
	acceptor->listenning = 1;
	int32_t ret = listen(acceptor->fd, SOMAXCONN);
	if(ret < 0){
		exit(-1);
	}
	enable_reading(acceptor->accept_handler);
}
