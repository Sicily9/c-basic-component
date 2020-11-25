#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "gp.h"
#include "proto/test.pb-c.h"

int main(int argc,char *argv[])
{
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
		
#if 1
		Name a;
		name__init(&a);
		a.name = malloc(10);
		strcpy(a.name, "i am a");

		a.id = 1;
		a.age = 3;

		uint8_t *msg = NULL;
		int32_t size = encode((ProtobufCMessage *)&a, &msg);
		printf("msg: %s\n", msg);

		n = send(sockfd, msg, size,0); 
		if(n < 0) {
			perror("send");
 			exit(1);
 		}else{
			printf("send msg:%s state:%ld-%s\n", msg, n, strerror(errno));
			free(msg);
		}
#endif

#if 1
		char buf[40] = {0};
		n = recv(sockfd, buf, sizeof buf, 0); 
		if(n < 0) {
			perror("receive");
 			exit(1);
 		}else{
			printf("recv msg:%s state:%ld-%s\n", buf, n, strerror(errno));
			memset(buf, 0, sizeof buf);
		}
#endif
		sleep(1);
	}
	return 0;

}
