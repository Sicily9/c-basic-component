#include "gp.h"

dict *callback_map = NULL;

static dict *get_callback_map(void)
{
	if(callback_map == NULL){
		callback_map = dictCreate(STRING_DICT, 0);
	}
	return callback_map;
}


void register_msg_callback(char *name, gp_protobuf_msg_callback callback)
{
	dict *map = get_callback_map();
	dictAdd(map, name, callback);
}

gp_protobuf_msg_callback get_msg_callback(const char *name)
{
	dict *map = get_callback_map();
	return dictFetchValue(map, name);
}
