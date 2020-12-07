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

		MatchOption match_option;
		match_option__init(&match_option);

		match_option.field = malloc(14);
		strcpy(match_option.field, "xssql-module");

		match_option.match_rule = 92;

		match_option.expression = malloc(15);
		strcpy(match_option.expression, "a!=1 && b==2");


		MatchOption match_option2;
		match_option__init(&match_option2);

		match_option2.field = malloc(11);
		strcpy(match_option2.field, "cc-module");

		match_option2.match_rule = 93;

		match_option2.expression = malloc(15);
		strcpy(match_option2.expression, "c!=4 && d==3");

		DefineRule define_rule;
		define_rule__init(&define_rule);

		define_rule.id = 1;
		define_rule.seq = 2;
		define_rule.mode = 3;
		define_rule.enable = 4;
		define_rule.level = 5;
		define_rule.log_type = 6;
		define_rule.n_match_option_list = 2;

		define_rule.match_option_list = malloc(define_rule.n_match_option_list * sizeof(MatchOption *));
		define_rule.match_option_list[0] = &match_option;
		define_rule.match_option_list[1] = &match_option2;

		define_rule.name = malloc(6);
		strcpy(define_rule.name, "test1");

		define_rule.expire_time = malloc(11);
		strcpy(define_rule.expire_time, "2020:12:1");

		uint8_t *msg = NULL;
		int32_t size = 0;
        encode((ProtobufCMessage *)&define_rule, &msg, &size);

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
