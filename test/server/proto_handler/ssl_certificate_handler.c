#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t ssl_certificate_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	SslCertificate *ssl_certificate = (SslCertificate *)msg;

	printf("ssl_certificate:{id:%d, cert_name:%s, enable:%d, cert_path:%s, key_path:%s, cert_password:%s}\n", ssl_certificate->id, ssl_certificate->cert_name, ssl_certificate->enable, ssl_certificate->cert_path, ssl_certificate->key_path, ssl_certificate->cert_password);

	char *str ="i receive the ssl_certificate msg";
	printf("fd:%d, send msg:%s, len:%d\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_ssl_certificate_handler(void)
{
	register_msg_callback("SslCertificate", ssl_certificate_handler);
	register_name_pb_map("SslCertificate", &ssl_certificate__descriptor);
}

gp_module_init(ssl_certificate_handler, MODULE_INIT_FIRST, "ssl_certificate_handler", NULL, early_init_ssl_certificate_handler);
