#include "proto.h"
#include "gp.h"

static dict *dict1 = NULL;

void early_init_proto(void)
{
	printf("proto init\n");
	dict1 = dictCreate(STRING_DICT, 0);
	dictAdd(dict1, "Name", (void *)&name__descriptor);
}

size_t encode(ProtobufCMessage * msg, unsigned char **out)
{
	int t = protobuf_c_message_get_packed_size(msg);
	unsigned char *tmp = malloc(t);
	*out = tmp;
	int size = protobuf_c_message_pack(msg, *out);
	return size;
}

ProtobufCMessage * decode(char * name, size_t len, unsigned char *data)
{
	ProtobufCMessageDescriptor* desc = dictFetchValue(dict1, name);
	ProtobufCMessage *msg =protobuf_c_message_unpack(desc, NULL, len, data);
	free(data);
	return msg;
}

gp_module_desc proto_module; 

gp_module_init(proto_module, MODULE_INIT_FIRST, "proto", NULL, early_init_proto);
