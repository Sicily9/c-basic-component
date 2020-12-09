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

int32_t send_all_sync(int32_t fd, char *msg, int32_t *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    
    while(total < *len) {
        n = write(fd, msg + total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int32_t read_all_sync(int32_t fd, char *msg, int32_t *len)
{
    int32_t total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    
    while(total < *len) {
        n = read(fd, msg + total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
