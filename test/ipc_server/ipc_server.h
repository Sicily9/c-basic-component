#include "gp.h"

typedef struct ipc_server_s ipc_server;

struct ipc_server_s
{
	gp_loop *loop;
	gp_server *server;
};


extern void init_ipc_server(ipc_server *, gp_loop *, gp_sock_address *, char *);
extern void create_ipc_server(ipc_server **, gp_loop *, gp_sock_address *, char *);
extern void ipc_start_server(ipc_server *);


