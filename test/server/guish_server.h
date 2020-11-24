#include "gp.h"

typedef struct guish_server_s guish_server;

struct guish_server_s
{
	gp_loop *loop;
	gp_tcp_server *tcp_server;
};

extern void init_guish_server(guish_server *, gp_loop *, gp_inet_address *, char *);
extern void create_guish_server(guish_server **, gp_loop *, gp_inet_address *, char *);
extern void guish_start_server(guish_server *);

