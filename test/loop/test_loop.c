#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "gp.h"

void timer_func1(void *data)
{
	printf("i am 1\n");
}

void timer_func2(void *data)
{
	printf("i am 2\n");
}

void listen_cb(struct gp_loop_s *loop, struct gp_io_s *w, unsigned int events)
{
	printf("listen_cb: i am listening\n");
}

int main()
{
    struct sockaddr_in addr;
  	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(8888);
	inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);

    bind(sock,(struct sockaddr*)&addr,sizeof(addr));
    listen(sock,5);

	gp_io *w=NULL;
	create_gp_io(&w, listen_cb, sock);

	gp_loop *loop = NULL;
	create_gp_loop(&loop);
	
	gp_io_start(loop, w, EPOLLIN);
	gp_loop_timer_start(loop, timer_func1, NULL, 500, 1);
	gp_loop_timer_start(loop, timer_func2, NULL, 1000, 0);
	

	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
