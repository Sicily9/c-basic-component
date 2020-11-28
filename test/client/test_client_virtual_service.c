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

		AttackDetect attack_detect3;
		attack_detect__init(&attack_detect3);
		attack_detect3.enable = 7;
		attack_detect3.block = 8;
		attack_detect3.log = 9;

		attack_detect3.name = malloc(6);
		strcpy(attack_detect3.name, "test3");


		AttackDetect attack_detect4;
		attack_detect__init(&attack_detect4);
		attack_detect4.enable = 10;
		attack_detect4.block = 11;
		attack_detect4.log = 12;

		attack_detect4.name = malloc(6);
		strcpy(attack_detect4.name, "test4");

		SiteSecurity site_security2;
		site_security__init(&site_security2);
		site_security2.id = 3;
		site_security2.block_type = 4;
		site_security2.n_attack_detect_list = 2;
		site_security2.attack_detect_list = malloc(site_security2.n_attack_detect_list * sizeof(AttackDetect *));
		site_security2.attack_detect_list[0] = &attack_detect3;
		site_security2.attack_detect_list[1] = &attack_detect4;

		site_security2.name = malloc(8);
		strcpy(site_security2.name, "default");

		site_security2.decode_type = malloc(7);
		strcpy(site_security2.decode_type, "base64");

		site_security2.http_req_body_detect = malloc(6);
		strcpy(site_security2.http_req_body_detect, "12345");

		
		Bizserver bizserver1;
		bizserver__init(&bizserver1);

		bizserver1.protocol = malloc(5);
		strcpy(bizserver1.protocol, "https");

		bizserver1.address = malloc(15);
		strcpy(bizserver1.address, "192.168.38.55");

		bizserver1.port = malloc(5);
		strcpy(bizserver1.port, "8065");

		Bizserver bizserver2;
		bizserver__init(&bizserver2);

		bizserver2.protocol = malloc(4);
		strcpy(bizserver2.protocol, "http");

		bizserver2.address = malloc(15);
		strcpy(bizserver2.address, "192.168.38.66");

		bizserver2.port = malloc(5);
		strcpy(bizserver2.port, "8077");


		SslCertificate ssl_certificate1;
		ssl_certificate__init(&ssl_certificate1);
		ssl_certificate1.id = 1;
		ssl_certificate1.enable = 1;

		ssl_certificate1.cert_name = malloc(10);
		strcpy(ssl_certificate1.cert_name, "test.cert");

		ssl_certificate1.cert_path = malloc(10);
		strcpy(ssl_certificate1.cert_path, "/tmp/cert");

		ssl_certificate1.key_path = malloc(9);
		strcpy(ssl_certificate1.key_path, "/tmp/key");

		ssl_certificate1.cert_password = malloc(7);
		strcpy(ssl_certificate1.cert_password, "123456");

		SslCertificate ssl_certificate2;
		ssl_certificate__init(&ssl_certificate2);
		ssl_certificate2.id = 2;
		ssl_certificate2.enable = 2;

		ssl_certificate2.cert_name = malloc(11);
		strcpy(ssl_certificate2.cert_name, "test2.cert");

		ssl_certificate2.cert_path = malloc(11);
		strcpy(ssl_certificate2.cert_path, "/tmp2/cert");

		ssl_certificate2.key_path = malloc(10);
		strcpy(ssl_certificate2.key_path, "/tmp2/key");

		ssl_certificate2.cert_password = malloc(7);
		strcpy(ssl_certificate2.cert_password, "123457");

		VirtualService virtual_service;
		virtual_service__init(&virtual_service);
		virtual_service.id = 1;
		virtual_service.enable = 2;
		virtual_service.ssl_enable = 3;
		virtual_service.response_type = 4;
		virtual_service.n_cw_ssl_certificate = 2;
		virtual_service.n_bizserver_list = 2;
		virtual_service.n_cw_site_security = 2;

		virtual_service.name = malloc(7);
		strcpy(virtual_service.name, "default");

		virtual_service.bind_domain = malloc(5);
		strcpy(virtual_service.bind_domain, "NULL");

		virtual_service.ports = malloc(5);
		strcpy(virtual_service.ports, "9999");

		virtual_service.ssl_version = malloc(8);
		strcpy(virtual_service.ssl_version, "TLSv1.2");

		virtual_service.ssl_cipher = malloc(5);
		strcpy(virtual_service.ssl_cipher, "NULL");

		virtual_service.cw_ssl_certificate = malloc(virtual_service.n_cw_ssl_certificate * sizeof(SslCertificate *));
		virtual_service.cw_ssl_certificate[0] = &ssl_certificate1;
		virtual_service.cw_ssl_certificate[1] = &ssl_certificate2;

		virtual_service.bizserver_list = malloc(virtual_service.n_bizserver_list * sizeof(Bizserver *));
		virtual_service.bizserver_list[0] = &bizserver1;
		virtual_service.bizserver_list[1] = &bizserver2;

		virtual_service.cw_site_security = malloc(virtual_service.n_cw_site_security * sizeof(SiteSecurity *));
		virtual_service.cw_site_security[0] = &site_security;
		virtual_service.cw_site_security[1] = &site_security2;

		uint8_t *msg = NULL;
		int32_t size = encode((ProtobufCMessage *)&virtual_service, &msg);

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
