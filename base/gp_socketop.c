#include "gp.h"

void get_local_address(int32_t fd, char a[], int *port, int len)
{
    struct sockaddr_in serv;
    getsockname(fd, (struct sockaddr *)&serv, (socklen_t *)&len);
    inet_ntop(AF_INET, &serv.sin_addr, a, (socklen_t)len);
	*port = ntohs(serv.sin_port);
}

void get_peer_address(int32_t fd, char a[], int *port, int len)
{
    struct sockaddr_in serv;
    getpeername(fd, (struct sockaddr *)&serv, (socklen_t *)&len);
    inet_ntop(AF_INET, &serv.sin_addr, a, (socklen_t)len);
	*port = ntohs(serv.sin_port);
}

void get_domain_local_address(int32_t fd, char a[])
{
    struct sockaddr_un serv;
    socklen_t len = sizeof(serv);
    getsockname(fd, (struct sockaddr *)&serv, (socklen_t *)&len);
    strcpy(a, serv.sun_path);
}

void get_domain_peer_address(int32_t fd, char a[])
{
    struct sockaddr_un serv;
    socklen_t len = sizeof(serv);
    getpeername(fd, (struct sockaddr *)&serv, &len);
    strcpy(a, serv.sun_path);
}
