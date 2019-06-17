#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "gp_loop.h"

void timer_func1(void *data)
{
	gp_loop *loop = data;
	printf("i am 1\n");
	gp_loop_timer_start(loop, timer_func1, loop, loop->time+1000);
}

void timer_func2(void *data)
{
	gp_loop *loop = data;
	printf("i am 2\n");
	gp_loop_timer_start(loop, timer_func2, loop, loop->time+2000);
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
	gp_loop_timer_start(loop, timer_func1, loop, loop->time+500);
	gp_loop_timer_start(loop, timer_func2, loop, loop->time+1000);
	

	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
