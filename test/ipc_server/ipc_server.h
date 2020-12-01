#include "gp.h"

typedef struct ipc_server_s ipc_server;
typedef struct transport_msg_s transport_msg;

struct ipc_server_s
{
	gp_loop *loop;
	gp_server *server;
};

struct transport_msg_s
{
	ProtobufCMessage *msg;
	gp_connection *conn;
};


extern gp_task_processor *get_task_processor(void);
extern void init_ipc_server(ipc_server *, gp_loop *, gp_sock_address *, char *);
extern void create_ipc_server(ipc_server **, gp_loop *, gp_sock_address *, char *);
extern void ipc_start_server(ipc_server *);

extern void init_transport_msg(transport_msg *, ProtobufCMessage *, gp_connection *);
extern void create_transport_msg(transport_msg **, ProtobufCMessage *, gp_connection *);

