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

ProtobufCMessage * decode(char *name, size_t len, void *data)
{
	ProtobufCMessageDescriptor* desc = dictFetchValue(pb_map, name);
	ProtobufCMessage *msg = protobuf_c_message_unpack(desc, NULL, len, data);

	return msg;
}

