#include "gp.h"
#include "ipc_server/proto_interface/cwaf_config_buf.pb-c.h"

uint32_t virtual_service_handler(gp_connection *conn, ProtobufCMessage *msg)
{
	VirtualService *virtual_service = (VirtualService *)msg;

	printf("virtual_service:{id:%d, name:%s, bind_domain:%s, enable:%d, ssl_enable:%d, ports:%s, ssl_version:%s, ssl_cipher:%s, response_type:%d, n_cw_ssl_certificate:%ld, n_bizserver_list:%ld, n_cw_site_security:%ld}\n", virtual_service->id, virtual_service->name, virtual_service->bind_domain, virtual_service->enable, virtual_service->ssl_enable, virtual_service->ports, virtual_service->ssl_version, virtual_service->ssl_cipher, virtual_service->response_type, virtual_service->n_cw_ssl_certificate, virtual_service->n_bizserver_list, virtual_service->n_cw_site_security);

	for(int i = 0; i < virtual_service->n_cw_ssl_certificate; i++){
		SslCertificate *ssl_certificate = virtual_service->cw_ssl_certificate[i];
		printf("ssl_certificate:{id:%d, cert_name:%s, enable:%d, cert_path:%s, key_path:%s, cert_password:%s}\n", ssl_certificate->id, ssl_certificate->cert_name, ssl_certificate->enable, ssl_certificate->cert_path, ssl_certificate->key_path, ssl_certificate->cert_password);
	}

	for(int i = 0; i < virtual_service->n_bizserver_list; i++){
		Bizserver *bizserver = virtual_service->bizserver_list[i];
		printf("bizserver:{protocol:%s, address:%s, port:%s}\n", bizserver->protocol, bizserver->address, bizserver->port);
	}

	for(int i = 0; i < virtual_service->n_cw_site_security; i++){
		SiteSecurity *site_security = virtual_service->cw_site_security[i];
		printf("site_security:{id:%d, name:%s, decode_type:%s, block_type:%d, n_attack_detect_list:%ld, http_req_body_detect:%s}\n", site_security->id, site_security->name, site_security->decode_type, site_security->block_type, site_security->n_attack_detect_list, site_security->http_req_body_detect);
		for(int j = 0; j < site_security->n_attack_detect_list; j++){
			AttackDetect *attack_detect = site_security->attack_detect_list[j];
			printf("attack_detect:{name:%s, enable:%d, block:%d, log:%d}\n", attack_detect->name, attack_detect->enable, attack_detect->block, attack_detect->log);
		}

	}

    VirtualServiceRes pb_msg;
    virtual_service_res__init(&pb_msg);
    pb_msg.id = 3;

    uint8_t *tmsg = NULL;
    int32_t size =0;
    encode((ProtobufCMessage *)&pb_msg, &tmsg, &size);
	conn_send(conn, (char *)tmsg, size);


#if 0
	char *str = "i receive the virtual_service msg";
	printf("fd:%d, send msg:%s, len:%ld\n", conn->fd, str, strlen(str) + 1);
	conn_send(conn, str, strlen(str) + 1);
#endif
	return 0;
}

void early_init_virtual_service_handler(void)
{
	register_msg_callback("VirtualService", virtual_service_handler);
	register_name_pb_map("VirtualService", &virtual_service__descriptor);
}

gp_module_init(virtual_service_handler, MODULE_INIT_FIRST, "virtual_service_handler", NULL, early_init_virtual_service_handler);
