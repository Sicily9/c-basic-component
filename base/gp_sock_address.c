#include "gp.h"
#include <strings.h>

void init_gp_sock_address(gp_sock_address *sock_address, char *address, uint16_t port, uint8_t type)
{
    if (type == IPV6){
        bzero(&(sock_address->addr6), sizeof sock_address->addr6);
        sock_address->addr6.sin6_family = AF_INET6;
        sock_address->addr6.sin6_port = htobe16(port);
        if (inet_pton(AF_INET6, address, &sock_address->addr6.sin6_addr) <= 0){

        }
    }else if (type == IP){
        bzero(&(sock_address->addr), sizeof sock_address->addr);
        sock_address->addr.sin_family = AF_INET;
        sock_address->addr.sin_port = htobe16(port);
        if (inet_pton(AF_INET, address, &sock_address->addr.sin_addr) <= 0){
            
        }
    }else if (type ==DOMAIN){
        bzero(&(sock_address->path), sizeof sock_address->path);
        sock_address->path.sun_family = AF_UNIX;
        strcpy(&sock_address->path.sun_path, address);
    }
}

void init_gp_sock_address_by_sockaddr(gp_sock_address *sock_address, struct sockaddr *addr)
{
    if(addr->sa_family == AF_INET){
        memcpy(&sock_address->addr, addr, sizeof(struct sockaddr_in));
    }else if(addr->sa_family == AF_INET6){
        memcpy(&sock_address->addr6, addr, sizeof(struct sockaddr_in6));
    }else if(addr->sa_family == AF_UNIX){
        memcpy(&sock_address->path, addr, sizeof(struct sockaddr_un));
    }
}


void create_gp_sock_address(gp_sock_address **sock_address, char *ip, uint16_t port, uint8_t type)
{
    gp_sock_address *tmp = malloc(sizeof(gp_sock_address));
    memset(tmp, 0, sizeof(*tmp));
    init_gp_sock_address(tmp, ip, port, type);
    *sock_address = tmp;
}


