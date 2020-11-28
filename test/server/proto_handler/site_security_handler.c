#include "server/guish_server.h"
#include "server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t site_security_handler(gp_tcp_connection *conn, ProtobufCMessage *msg)
{
	SiteSecurity *site_security = (SiteSecurity *)msg;

	printf("site_security:{id:%d, name:%s, decode_type:%s, block_type:%d, n_attack_detect_list:%ld, http_req_body_detect:%s}\n", site_security->id, site_security->name, site_security->decode_type, site_security->block_type, site_security->n_attack_detect_list, site_security->http_req_body_detect);

	for(int i = 0; i < site_security->n_attack_detect_list; i++){
		AttackDetect *attack_detect = site_security->attack_detect_list[i];
		printf("attack_detect:{name:%s, enable:%d, block:%d, log:%d}\n", attack_detect->name, attack_detect->enable, attack_detect->block, attack_detect->log);
	}

	char *str ="i receive the site_security msg";
	conn_send(conn, str, strlen(str) + 1);
	return 0;
}

void early_init_site_security_handler(void)
{
	register_msg_callback("SiteSecurity", site_security_handler);
	register_name_pb_map("SiteSecurity", &site_security__descriptor);
}

gp_module_init(site_security_handler, MODULE_INIT_FIRST, "site_security_handler", NULL, early_init_site_security_handler);
