#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "gp.h"
#include "proto_interface/cwaf_config_buf.pb-c.h"

int main(int argc,char *argv[])
{

	struct timeval start, end;  // define 2 struct timeval variables
 

    int sockfd;
    struct sockaddr_in their_addr;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(8000);
    their_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    bzero(&(their_addr.sin_zero), 8);
	connect(sockfd,(struct sockaddr*)&their_addr,sizeof(struct sockaddr));

    printf("Get the Server~Cheers!\n");
    while(1){
		size_t n = 0;

		StaticRoute static_route;
		static_route__init(&static_route);
		static_route.id = 1;

		static_route.interface_name = malloc(5);
		strcpy(static_route.interface_name, "eth1");

		static_route.dst_ip = malloc(15);
		strcpy(static_route.dst_ip, "192.168.38.203");

		static_route.subnet_mask = malloc(14);
		strcpy(static_route.subnet_mask, "255.255.255.0");

		static_route.gateway = malloc(13);
		strcpy(static_route.gateway, "192.168.38.1");

		uint8_t *msg = NULL;
		int32_t size = encode((ProtobufCMessage *)&static_route, &msg);

    	gettimeofday(&start, NULL); 
		n = send(sockfd, msg, size,0); 
		if(n < 0) {
			perror("send");
 			exit(1);
 		}else{
			printf("send msg:%s state:%ld-%s\n", msg, n, strerror(errno));
			free(msg);
		}

		char buf[40] = {0};
		n = recv(sockfd, buf, sizeof buf, 0); 
		if(n < 0) {
			perror("receive");
 			exit(1);
 		}else{
			printf("recv msg:%s state:%ld-%s\n", buf, n, strerror(errno));
			memset(buf, 0, sizeof buf);
		}
    	gettimeofday(&end, NULL); 
		long long total_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
		printf("pingpong: %lld us, %f ms\n", total_time, (double)total_time/1000.0);
		sleep(1);
	}
	return 0;

}
