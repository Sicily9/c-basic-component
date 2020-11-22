#include "gp.h"
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <endian.h>


void init_gp_inet_address(gp_inet_address *inet_address, char *ip, uint16_t port, uint8_t ipv6)
{
	if (ipv6){
		bzero(&(inet_address->addr6), sizeof inet_address->addr6);
		inet_address->addr6.sin6_family = AF_INET6;
        inet_address->addr6.sin6_port = htobe16(port);
        if (inet_pton(AF_INET6, ip, &inet_address->addr6.sin6_addr) <= 0){

		}
	}else{
		bzero(&(inet_address->addr), sizeof inet_address->addr);
		inet_address->addr.sin_family = AF_INET;
        inet_address->addr.sin_port = htobe16(port);
        if (inet_pton(AF_INET, ip, &inet_address->addr.sin_addr) <= 0){

		}
	}
}

void create_gp_inet_address(gp_inet_address **inet_address, char *ip, uint16_t port, uint8_t ipv6)
{
	gp_inet_address *tmp = malloc(sizeof(gp_inet_address));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_inet_address(tmp, ip, port, ipv6);
	*inet_address = tmp;
}


