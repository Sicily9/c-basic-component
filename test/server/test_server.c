#include "gp.h"
#include "test.pb-c.h"


void on_message(gp_tcp_connection *conn, gp_buffer *buffer)
{
	while (readable_bytes(buffer) >= 4)
    {
      void* data = peek(buffer);  
      int32_t be32 = *(const int32_t*)(data);

      const int32_t len = ntohl(be32);  //转换成主机字节序
      if (len > 65536 || len < 0)  //如果消息超过64K，或者长度小于0，不合法，干掉它。
      {
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
		char * name = malloc(name_len);
		memcpy(name, data, name_len);
      	retrieve(buffer, name_len); 

		data = peek(buffer);
		ProtobufCMessage *msg = decode(name, len - 4 - name_len, data);
		free(name);
        retrieve(buffer, len - 4 - name_len); 

		Name * t = (Name *)msg;
		printf("name: %s\n", t->name);
		protobuf_c_message_free_unpacked(msg, NULL);
      }
      else   //未达到一条完整的消息
      {
        break;  
      }
    }
}

void on_connection(gp_tcp_connection *conn)
{
}

void timer_func4(void *data)
{
    printf("i am 4\n");
}


int main()
{	
	printf("EPOLL_CTL_ADD:1 EPOLL_CTL_MOD:3 EPOLL_CTL_DEL:2\n");
	gp_loop *loop = NULL;
	create_gp_loop(&loop);

	gp_inet_address *address = NULL;
	create_gp_inet_address(&address, "0.0.0.0", 8000, 0);

	gp_tcp_server *server = get_tcp_server();
	init_gp_tcp_server(server, loop, address, "guish");
	
	server_set_message_callback(server, on_message);
	server_set_connection_callback(server, on_connection);
	register_name_pb_map("Name", &name__descriptor);

	start_server(server);

	//gp_loop_timer_start(loop, timer_func4, NULL, 2000, 1);
	
	gp_loop_run(loop, GP_RUN_DEFAULT);

	return 0;
}
