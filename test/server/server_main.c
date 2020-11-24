#include "guish_server.h"

int main()
{	
	printf("EPOLL_CTL_ADD:1 EPOLL_CTL_MOD:3 EPOLL_CTL_DEL:2\n");
	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_inet_address *address = NULL;
	create_gp_inet_address(&address, "0.0.0.0", 8000, 0);

	guish_server *server = NULL;
	create_guish_server(&server, loop, address, "guish");

	guish_start_server(server);
	
	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
