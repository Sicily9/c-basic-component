#include "proto.h"
#include "gp.h"

static dict *dict1 = NULL;

void early_init_proto(void)
{
	printf("proto init\n");
	dict1 = dictCreate(STRING_DICT, 0);
	dictAdd(dict1, "Name", (void *)&name__descriptor);
}

size_t encode(ProtobufCMessage * msg, uint8_t **out)
{
	int t = protobuf_c_message_get_packed_size(msg);
	uint8_t *tmp = malloc(t);
	*out = tmp;
	int size = protobuf_c_message_pack(msg, *out);
	return size;
}

ProtobufCMessage * decode(char * name, size_t len, uint8_t *data)
{
	ProtobufCMessageDescriptor* desc = dictFetchValue(dict1, name);
	ProtobufCMessage *msg =protobuf_c_message_unpack(desc, NULL, len, data);
	free(data);
	return msg;
}

gp_module_desc proto_module ={
    .name = "proto",
    .type = MODULE_INIT_FIRST,
    .early_init = early_init_proto,
};

gp_module_init(proto_module);
