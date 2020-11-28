#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t dns_config_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	DnsConfig *dns_config = (DnsConfig *)msg;

	printf("dns_config:{id:%d, interface_name:%s, host_name:%s, preferred_server:%s, alternative_server:%s}\n", dns_config->id, dns_config->interface_name, dns_config->host_name, dns_config->preferred_server, dns_config->alternative_server);

	char *str = "i receive the dns_config msg";
	printf("fd:%ld, send msg:%s, len:%d\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_dns_config_handler(void)
{
	register_msg_callback("DnsConfig", dns_config_handler);
	register_name_pb_map("DnsConfig", &dns_config__descriptor);
}

gp_module_init(dns_config_handler, MODULE_INIT_FIRST, "dns_config_handler", NULL, early_init_dns_config_handler);
