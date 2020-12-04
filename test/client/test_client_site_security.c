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

		AttackDetect attack_detect;
		attack_detect__init(&attack_detect);
		attack_detect.enable = 1;
		attack_detect.block = 2;
		attack_detect.log = 3;

		attack_detect.name = malloc(5);
		strcpy(attack_detect.name, "test");

		AttackDetect attack_detect2;
		attack_detect__init(&attack_detect2);
		attack_detect2.enable = 4;
		attack_detect2.block = 6;
		attack_detect2.log = 7;

		attack_detect2.name = malloc(6);
		strcpy(attack_detect2.name, "test2");

		SiteSecurity site_security;
		site_security__init(&site_security);
		site_security.id = 1;
		site_security.block_type = 2;
		site_security.n_attack_detect_list = 2;
		site_security.attack_detect_list = malloc(site_security.n_attack_detect_list * sizeof(AttackDetect *));
		site_security.attack_detect_list[0] = &attack_detect;
		site_security.attack_detect_list[1] = &attack_detect2;

		site_security.name = malloc(8);
		strcpy(site_security.name, "default");

		site_security.decode_type = malloc(7);
		strcpy(site_security.decode_type, "base64");

		site_security.http_req_body_detect = malloc(6);
		strcpy(site_security.http_req_body_detect, "12345");

		uint8_t *msg = NULL;
		int32_t size = 0;
        encode((ProtobufCMessage *)&site_security, &msg, &size);

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
