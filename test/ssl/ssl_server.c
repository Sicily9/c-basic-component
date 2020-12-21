#include "gp.h"

#define LOCAL_CERT "server.pem"
#define LOCAL_KEY  "privkey.pem"

int main()
{	
	printf("EPOLL_CTL_ADD:1 EPOLL_CTL_MOD:3 EPOLL_CTL_DEL:2\n");
	printf("POLLHUP:16 POLLIN:1 POLLERR:8 POLLNVAL:32 POLLPRI:2 POLLRDHUP:8192 POLLOUT:4\n");

	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_sock_address *address = NULL;
	create_gp_sock_address(&address, "0.0.0.0", 8443, IP);

	gp_ssl_server *server = NULL;
	create_gp_ssl_server(&server, loop, address, "ssl");
    gp_set_ssl_cert_key(server, LOCAL_CERT, LOCAL_KEY);

	gp_ssl_start_server(server);
	
	gp_loop_run(loop, GP_RUN_DEFAULT);


	return 0;
}