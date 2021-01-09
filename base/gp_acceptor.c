#include "gp.h"

gp_acceptor * get_acceptor_from_handler(gp_handler *handler){
        return container_of(handler, gp_acceptor, accept_handler);
}

static void acceptor_read_callback(gp_handler *handler)
{
        gp_acceptor * acceptor = get_acceptor_from_handler(handler);

        struct sockaddr addr;
        bzero(&addr, sizeof addr);
        socklen_t addrlen = sizeof(struct sockaddr_un);
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
                acceptor->new_connection_callback(acceptor, connfd, &addr);
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
        init_gp_handler(&acceptor->accept_handler, loop, acceptor->fd);

        int32_t optval = 1;
        setsockopt(acceptor->fd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)(sizeof optval));

        int len = get_gp_sock_len(sock_address);
        char address[40];
        get_gp_sock_address(sock_address, address, 40);
        if(((struct sockaddr *)sock_address)->sa_family == AF_UNIX){
                unlink(address);
        }

        if(bind(acceptor->fd, (struct sockaddr*)&(sock_address->addr), len) < 0 ){
                printf("size:%ld, address:%s, errno:%d, %s\n",sizeof(*sock_address), address, errno, strerror(errno));
                abort();
        }
        printf("bind address:%s\n", address);

        set_read_callback(&acceptor->accept_handler, acceptor_read_callback);
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

        if(ret < 0){
                printf("errno:%d, %s\n",errno, strerror(errno));
                abort();
        }
        enable_reading(&acceptor->accept_handler);
}
