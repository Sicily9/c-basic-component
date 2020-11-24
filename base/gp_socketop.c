#include "gp.h"

void get_local_address(int32_t fd, char a[], int *port, int len)
{
    struct sockaddr_in serv;
    getpeername(fd, (struct sockaddr *)&serv, (socklen_t *)&len);
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
