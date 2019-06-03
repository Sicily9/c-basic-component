#include "task.pb-c.h"

extern size_t encode(ProtobufCMessage * msg, uint8_t **out);
extern ProtobufCMessage * decode(char * name, size_t len, uint8_t *data);
