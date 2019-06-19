#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
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

void timer_func3(void *data)
{
	printf("i am 3\n");
}

void timer_func4(void *data)
{
	printf("i am 4\n");
}

void timer_func5(void *data)
{
	printf("i am 5\n");
}

void timer_func6(void *data)
{
	printf("i am 6\n");
}

void listen_cb(struct gp_loop_s *loop, struct gp_io_s *w, unsigned int events)
{
	//accept(w->fd, NULL, NULL);//if not accept ,epoll would loop no limits
	printf("listen_cb: i am listening\n");
}

int set_nonblock(int fd) 
{   
      /* 获取文件的flags */
      int flags = fcntl(fd, F_GETFL);
      /* 设置文件的flags 为非阻塞*/
      fcntl(fd, F_SETFL, flags | O_NONBLOCK);
      return flags; 
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
	set_nonblock(sock);
	listen(sock,5);

	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_io *w=NULL;
	create_gp_io(&w, listen_cb, sock);
	gp_io_start(loop, w, EPOLLIN|EPOLLET);

	//gp_loop_timer_start(loop, timer_func1, NULL, 500, 1);
	//gp_loop_timer_start(loop, timer_func2, NULL, 1000, 1);
	//gp_loop_timer_start(loop, timer_func3, NULL, 1500, 1);
	//gp_loop_timer_start(loop, timer_func4, NULL, 2000, 1);
	//gp_loop_timer_start(loop, timer_func5, NULL, 2500, 1);
	//gp_loop_timer_start(loop, timer_func6, NULL, 3000, 1);
	

	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
