#include "gp.h"

const int k_header = 4;
static dict * pb_map = NULL;

static dict* get_pb_map(void)
{
	if(pb_map == NULL){
		pb_map = dictCreate(STRING_DICT, 0);
	}
	return pb_map;
}

void register_name_pb_map(char *name, const ProtobufCMessageDescriptor *desc)
{
	dict *map = get_pb_map();
	dictAdd(map, name, (void *)desc);
	printf("register_name_pb_map name:%s\n", name);
}

size_t encode_errcode(int32_t errcode, uint8_t **buf)
{
	int32_t len = k_header + k_header;

	uint8_t *str = malloc(len);
	*buf = str;

	int32_t be32 = htonl(0x1343EA4);
	memcpy(str, (int8_t *)&be32, sizeof(int32_t));
    
	be32 = htonl(errcode);
	memcpy(str + k_header, (int8_t *)&be32, sizeof(int32_t));

	return len;
}


size_t encode(ProtobufCMessage *msg, uint8_t **buf)
{
	const char * name = msg->descriptor->name;
	int t = protobuf_c_message_get_packed_size(msg);
	int32_t name_len = strlen(name) + 1;
	int32_t len = k_header + k_header + k_header + name_len + t;

	uint8_t *str = malloc(len);
	*buf = str;

	int32_t be32 = htonl(0x1343EA4);
	memcpy(str, (int8_t *)&be32, sizeof(int32_t));

	be32 = htonl(len - k_header - k_header);
	memcpy(str + k_header, (int8_t *)&be32, sizeof(int32_t));

	be32 = htonl(name_len);
	memcpy(str + k_header + k_header, (int8_t *)&be32, sizeof(int32_t));

	memcpy(str + k_header + k_header + k_header, name, name_len + 1);

	protobuf_c_message_pack(msg, str + k_header + k_header + k_header + name_len);
	
	return len;
}

ProtobufCMessage * decode(gp_buffer *buffer)
{
	void* data = peek(buffer);  
    int32_t be32 = *(const int32_t*)(data);
	int32_t magic = ntohl(be32);
	if(unlikely(magic != 0x1343EA4))
	{
        retrieve_all(buffer);  
		printf("unknown message, don't handle magic:%d\n", magic);
		return NULL;
	}

	data = peek(buffer) + 4;  
    be32 = *(const int32_t*)(data);
    const int32_t len = ntohl(be32); 
    if (len > 65536 || len < 0)  {   
        retrieve_all(buffer);  
		return NULL;
    }   
    else if ((readable_bytes(buffer)) >= len + 8)  
    {                                                    
        retrieve(buffer, 8);  
        data = peek(buffer);
        be32 = *(const int32_t*)(data); 
        const int32_t name_len = ntohl(be32);
        retrieve(buffer, 4);  
            
        data = peek(buffer);
		char * name = calloc(1, name_len);
        memcpy(name, data, name_len);
        retrieve(buffer, name_len); 
		ProtobufCMessageDescriptor* desc = dictFetchValue(get_pb_map(), name);
		if(unlikely(desc == NULL)){
			printf("%s msg, haven't registered we don't handle\n", name);
        	retrieve_all(buffer);  
			return NULL;
		}
        free(name);

        data = peek(buffer);
		ProtobufCMessage *msg = protobuf_c_message_unpack(desc, NULL, len - 4 - name_len, data);
        retrieve(buffer, len - 4 - name_len); 
		return msg;
		
	} else {
        retrieve_all(buffer);  
        return NULL;  
    }
}

