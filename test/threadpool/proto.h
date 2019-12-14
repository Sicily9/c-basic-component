#include "task.pb-c.h"

extern size_t encode(ProtobufCMessage * msg, unsigned char **out);
extern ProtobufCMessage * decode(char * name, size_t len, unsigned char *data);
