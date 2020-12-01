#include "gp.h"
#include <strings.h>

void get_gp_sock_address(gp_sock_address *sock_address, char address[], int len)
{
    struct sockaddr *addr = (struct sockaddr *)sock_address;
    if(addr->sa_family == AF_INET){
        if (inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, address, len) <= 0){
        }
    }else if(addr->sa_family == AF_INET6){
        if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, address, len) <= 0){
        }
    }else if(addr->sa_family == AF_UNIX){
        if(len > sizeof((struct sockaddr_un *)addr)->sun_path)
            len = sizeof((struct sockaddr_un *)addr)->sun_path;
        strncpy(address, ((struct sockaddr_un *)addr)->sun_path, len);
    }
}

int32_t get_gp_sock_len_by_fd(int32_t fd)
{
    struct sockaddr addr;
    int len = sizeof(struct sockaddr);
    getsockname(fd, &addr, (socklen_t *)&len);

    if(addr.sa_family == AF_INET){
        return sizeof(struct sockaddr_in);
    }else if(addr.sa_family == AF_INET6){
        return sizeof(struct sockaddr_in6);
    }else if(addr.sa_family == AF_UNIX){
        return sizeof(struct sockaddr_un);
    }
}

int32_t get_gp_sock_len_by_sockaddr(struct sockaddr *addr)
{
    if(addr->sa_family == AF_INET){
        return sizeof(struct sockaddr_in);
    }else if(addr->sa_family == AF_INET6){
        return sizeof(struct sockaddr_in6);
    }else if(addr->sa_family == AF_UNIX){
        return sizeof(struct sockaddr_un);
    }
}

int32_t get_gp_sock_len(gp_sock_address *sock_address)
{
    struct sockaddr *addr = (struct sockaddr *)sock_address;
    if(addr->sa_family == AF_INET){
        return sizeof(struct sockaddr_in);
    }else if(addr->sa_family == AF_INET6){
        return sizeof(struct sockaddr_in6);
    }else if(addr->sa_family == AF_UNIX){
        return offsetof(struct sockaddr_un, sun_path) + strlen(sock_address->path.sun_path);
    }
}

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
    }else if (type == DOMAIN){
        bzero(&(sock_address->path), sizeof sock_address->path);
        sock_address->path.sun_family = AF_UNIX;
        strcpy(sock_address->path.sun_path, address);
    }
}

void init_gp_sock_address_by_sockaddr(gp_sock_address *sock_address, struct sockaddr *addr)
{
    if(addr->sa_family == AF_INET){
        memcpy(&sock_address->addr, addr, sizeof(struct sockaddr_in));
    }else if(addr->sa_family == AF_INET6){
        memcpy(&sock_address->addr6, addr, sizeof(struct sockaddr_in6));
    }else if(addr->sa_family == AF_UNIX){
        sock_address->path.sun_family = AF_UNIX;
        memcpy(sock_address->path.sun_path, ((struct sockaddr_un *)addr)->sun_path, strlen(((struct sockaddr_un *)addr)->sun_path));
        printf("%d\n", offsetof(struct sockaddr_un, sun_path) + strlen(((struct sockaddr_un *)addr)->sun_path));
    }
}


void create_gp_sock_address(gp_sock_address **sock_address, char *address, uint16_t port, uint8_t type)
{
    gp_sock_address *tmp = malloc(sizeof(gp_sock_address));
    memset(tmp, 0, sizeof(*tmp));
    init_gp_sock_address(tmp, address, port, type);
    *sock_address = tmp;
}


struct sockaddr * create_sockaddr(struct sockaddr * addr)
{
    if(addr->sa_family == AF_INET){
        return (struct sockaddr *)calloc(1, sizeof(struct sockaddr_in));
    }else if(addr->sa_family == AF_INET6){
        return (struct sockaddr *)calloc(1, sizeof(struct sockaddr_in6));
    }else if(addr->sa_family == AF_UNIX){
        return (struct sockaddr *)calloc(1, sizeof(struct sockaddr_un));
    }
}
