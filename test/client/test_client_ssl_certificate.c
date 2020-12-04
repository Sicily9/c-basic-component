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

		SslCertificate ssl_certificate;
		ssl_certificate__init(&ssl_certificate);
		ssl_certificate.id = 1;
		ssl_certificate.enable = 1;

		ssl_certificate.cert_name = malloc(10);
		strcpy(ssl_certificate.cert_name, "test.cert");

		ssl_certificate.cert_path = malloc(10);
		strcpy(ssl_certificate.cert_path, "/tmp/cert");

		ssl_certificate.key_path = malloc(9);
		strcpy(ssl_certificate.key_path, "/tmp/key");

		ssl_certificate.cert_password = malloc(7);
		strcpy(ssl_certificate.cert_password, "123456");

		uint8_t *msg = NULL;
		int32_t size = 0;
        encode((ProtobufCMessage *)&ssl_certificate, &msg, &size);

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
