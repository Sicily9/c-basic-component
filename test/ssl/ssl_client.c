#include "gp.h"
#include "ssl/proto_interface/cwaf_config_buf.pb-c.h"
 
#define MAXBUF 1024
 
void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("数字证书信息:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("证书: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("颁发者: %s\n", line);
        free(line);
       X509_free(cert);
    } else
        printf("无证书信息！\n");
}
 
int main(int argc, char **argv)
{
    int sockfd, len;
    struct sockaddr_in dest;
    char buffer[MAXBUF + 1];
    SSL_CTX *ctx;
    SSL *ssl;
 
    if (argc != 3) {
        printf("参数格式错误！正确用法如下：\n\t\t%s IP地址 端口\n\t比如:\t%s 127.0.0.1 80\n此程序用来从某个"
             "IP 地址的服务器某个端口接收最多 MAXBUF 个字节的消息",
             argv[0], argv[0]);
        exit(0);
    }
 
    /* SSL 库初始化 */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
 
    /* 创建一个 socket 用于 tcp 通信 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }
    printf("socket created\n");
 
    /* 初始化服务器端（对方）的地址和端口信息 */
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }
    printf("address created\n");
 
    /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    printf("server connected\n");
    /* 基于 ctx 产生一个新的 SSL */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    /* 建立 SSL 连接 */
    SSL_set_connect_state(ssl);
    int rc = SSL_do_handshake(ssl);
    if (rc == -1){
        int t_Error = SSL_get_error(ssl,rc);
        printf("%d\n", t_Error);
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }

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
		int32_t size = 0;
        encode((ProtobufCMessage *)&virtual_service, &msg, &size);

		n = SSL_write(ssl, msg, size); 
		if(n < 0) {
			perror("send");
 			exit(1);
 		}else{
			printf("send msg:%s state:%ld-%s\n", msg, n, strerror(errno));
			free(msg);
		}
        sleep(1);
    }

    return 0;
}