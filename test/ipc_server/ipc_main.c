#include "gp.h"

int main()
{	
	printf("EPOLL_CTL_ADD:1 EPOLL_CTL_MOD:3 EPOLL_CTL_DEL:2\n");
	printf("POLLHUP:16 POLLIN:1 POLLERR:8 POLLNVAL:32 POLLPRI:2 POLLRDHUP:8192 POLLOUT:4\n");

    gp_ipc_start();

#if 0
	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_sock_address *address = NULL;
	create_gp_sock_address(&address, "/tmp/ipc_server2.socket", -1, DOMAIN);

	ipc_server *server = NULL;
	create_ipc_server(&server, loop, address, "ipc");

	ipc_start_server(server);
	
	gp_loop_run(loop);
#endif

	return 0;
}
