#include "gp.h"

typedef struct guish_server_s guish_server;
typedef struct transport_msg_s transport_msg;

struct guish_server_s
{
	gp_loop *loop;
	gp_server server;
};

struct transport_msg_s
{
	ProtobufCMessage *msg;
	gp_connection *conn;
};


extern gp_task_processor *get_task_processor(void);
extern void init_guish_server(guish_server *, gp_loop *, gp_sock_address *, char *);
extern void create_guish_server(guish_server **, gp_loop *, gp_sock_address *, char *);
extern void guish_start_server(guish_server *);

extern void init_transport_msg(transport_msg *, ProtobufCMessage *, gp_connection *);
extern void create_transport_msg(transport_msg **, ProtobufCMessage *, gp_connection *);

