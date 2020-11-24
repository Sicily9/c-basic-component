#include "guish_server.h"

void on_message(gp_tcp_connection *conn, gp_buffer *buffer)
{
    while (readable_bytes(buffer) >= 4)
    {   
      void* data = peek(buffer);  
      int32_t be32 = *(const int32_t*)(data);
      const int32_t len = ntohl(be32); 
      if (len > 65536 || len < 0)  {   
        break;
      }   
      else if ((readable_bytes(buffer)) >= len + 4)  
      {                                                    
        retrieve(buffer, 4);  
        data = peek(buffer);
        be32 = *(const int32_t*)(data); 
        const int32_t name_len = ntohl(be32);
        retrieve(buffer, 4);  
            
        data = peek(buffer);
		char * name = calloc(1, name_len);
        memcpy(name, data, name_len);
        retrieve(buffer, name_len); 

        data = peek(buffer);
        ProtobufCMessage *msg = decode(name, len - 4 - name_len, data);
        free(name);
        retrieve(buffer, len - 4 - name_len); 


		gp_protobuf_msg_callback cb = get_msg_callback(msg->descriptor->name);
		cb(conn, msg);

        protobuf_c_message_free_unpacked(msg, NULL);
      }
      else   
      {
        break;  
      }
    }
}

void on_connection(gp_tcp_connection *conn)
{

}

void guish_start_server(guish_server *server)
{
	start_server(server->tcp_server);
}

void init_guish_server(guish_server *server, gp_loop *loop, gp_inet_address *address, char *name)
{
	gp_tcp_server *tcp_server = get_tcp_server();
	server->loop = loop;
	server->tcp_server = tcp_server;
	init_gp_tcp_server(tcp_server, loop, address, "guish");
	
	server_set_message_callback(tcp_server, on_message);
	server_set_connection_callback(tcp_server, on_connection);
}

void create_guish_server(guish_server **server, gp_loop *loop, gp_inet_address *address, char *name)
{
	guish_server *tmp = malloc(sizeof(guish_server));
	memset(tmp, 0, sizeof(guish_server));
	init_guish_server(tmp, loop, address, name);
	*server = tmp;
}
