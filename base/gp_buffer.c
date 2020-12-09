#include "gp.h"
#include <errno.h>
#include <stdlib.h>
#include <sys/uio.h>

const int32_t k_cheap_prepend = 8;
const int32_t k_initial_size = 8192;

void init_gp_buffer(gp_buffer *buffer)
{
	buffer->buffer = malloc(k_cheap_prepend + k_initial_size);
	buffer->len = k_cheap_prepend + k_initial_size;
	buffer->reader_index = k_cheap_prepend;
	buffer->writer_index = k_cheap_prepend;
}

void create_gp_buffer(gp_buffer **buffer) {
	gp_buffer *tmp = malloc(sizeof(gp_buffer));
	init_gp_buffer(tmp);
	*buffer = tmp;
}

void destruct_gp_buffer(gp_buffer *buffer)
{
	free(buffer->buffer);
	free(buffer);
}

size_t readable_bytes(gp_buffer *buffer)
{
	return buffer->writer_index - buffer->reader_index;
}

size_t writable_bytes(gp_buffer *buffer)
{
	return buffer->len - buffer->writer_index;
}

size_t prependable_bytes(gp_buffer *buffer)
{ 
	return buffer->reader_index; 
}

char* peek(gp_buffer *buffer)
{ 
	return buffer->buffer + buffer->reader_index; 
}


void retrieve_all(gp_buffer *buffer)
{
	buffer->reader_index = k_cheap_prepend;
	buffer->writer_index = k_cheap_prepend;
}

void has_written(gp_buffer *buffer, size_t len)
{
	buffer->writer_index += len;
}

void retrieve(gp_buffer *buffer, size_t len)
{
	if (len < readable_bytes(buffer))
    {
    	buffer->reader_index += len;
    }
    else
    {
        retrieve_all(buffer);
    }
}

char * retrieve_as_string(gp_buffer *buffer, size_t len)
{
	char *tmp = malloc(len); // rememeber free
	memcpy(tmp, peek(buffer), len);
	retrieve(buffer, len);
	return tmp;
}

char *retrieve_all_as_string(gp_buffer *buffer)
{
	return retrieve_as_string(buffer, readable_bytes(buffer));
}

static void make_space(gp_buffer *buffer, size_t len)
{
	if(writable_bytes(buffer) + prependable_bytes(buffer) < len + k_cheap_prepend){
		char *tmp = realloc(buffer->buffer, buffer->writer_index + len);
		buffer->buffer = tmp;
		buffer->len = len + buffer->writer_index;
	}else{
		size_t readable = readable_bytes(buffer);

		memmove(buffer->buffer + k_cheap_prepend, buffer->buffer + buffer->reader_index, readable_bytes(buffer));
		buffer->reader_index = k_cheap_prepend;
		buffer->writer_index = buffer->reader_index + readable;
	}
}

static void ensure_writable_bytes(gp_buffer *buffer, size_t len)
{
	if(writable_bytes(buffer) < len)
	{
		make_space(buffer, len);
	}
}

void buffer_append(gp_buffer *buffer, char * str, size_t len)
{
	ensure_writable_bytes(buffer, len);
	memcpy(buffer->writer_index + buffer->buffer, str, len);
	has_written(buffer, len);
}

size_t buffer_read_fd(gp_buffer *buffer, int32_t fd, int *saved_errno)
{
	  // saved an ioctl()/FIONREAD call to tell how much to read
	char extrabuf[65536];                                                         
  	struct iovec vec[2];
  	const size_t writable = writable_bytes(buffer);
  	vec[0].iov_base = buffer->buffer+buffer->writer_index;
  	vec[0].iov_len = writable;
  	vec[1].iov_base = extrabuf;
  	vec[1].iov_len = sizeof extrabuf;
  	// when there is enough space in this buffer, don't read into extrabuf.
  	// when extrabuf is used, we read 128k-1 bytes at most.
  	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  	const ssize_t n = readv(fd, vec, iovcnt);
  	if (n < 0)
  	{
    	*saved_errno = errno;
		printf("errno:%d, %s\n", errno, strerror(errno)); 
  	}
  	else if ((size_t)(n) <= writable)
  	{
    	buffer->writer_index += n;
  	}
  	else
  	{
    	buffer->writer_index = buffer->len;
    	buffer_append(buffer, extrabuf, n - writable);
  	}
  	return n;
}

ProtobufCMessage* peek_pb_msg(gp_buffer *buffer, void *desc, int32_t len)
{
        if(likely(readable_bytes(buffer) >= len)){
                ProtobufCMessage* msg = protobuf_c_message_unpack(desc, NULL, len, (const uint8_t *)peek(buffer));
                return msg;
        } else {
                abort();
        }
}

char* peek_str(gp_buffer *buffer, int32_t len)
{
        if(likely(readable_bytes(buffer) >= len)){
                char *msg = malloc(len);
                memcpy(msg, peek(buffer), len);
                return msg;
        } else {
                abort();
        }
}

int64_t peek_int64(gp_buffer *buffer)
{
        if(likely(readable_bytes(buffer) >= sizeof(int64_t))){
                int64_t be64 = 0;
                memcpy(&be64, peek(buffer), sizeof be64);
                return be64toh(be64);
        } else {
                abort();
        }
}

int32_t peek_int32(gp_buffer *buffer)
{
        if(likely(readable_bytes(buffer) >= sizeof(int32_t))){
                int32_t be32 = 0;
                memcpy(&be32, peek(buffer), sizeof be32);
                return be32toh(be32);
        } else {
                abort();
        }
}

int16_t peek_int16(gp_buffer *buffer)
{
        if(likely(readable_bytes(buffer) >= sizeof(int16_t))){
                int16_t be16 = 0;
                memcpy(&be16, peek(buffer), sizeof be16);
                return be16toh(be16);
        } else {
                abort();
        }
}

int8_t peek_int8(gp_buffer *buffer)
{
        if(likely(readable_bytes(buffer) >= sizeof(int8_t))){
                int8_t x = *peek(buffer);
                return x;
        } else {
                abort();
        }
}

int64_t read_int64(gp_buffer *buffer)
{
        int64_t result = peek_int64(buffer);
        retrieve(buffer, sizeof(int64_t));
        
        return result;
}

int32_t read_int32(gp_buffer *buffer)
{
        int32_t result = peek_int32(buffer);
        retrieve(buffer, sizeof(int32_t));
        
        return result;
}

int16_t read_int16(gp_buffer *buffer)
{
        int16_t result = peek_int16(buffer);
        retrieve(buffer, sizeof(int16_t));
        
        return result;
}

int8_t read_int8(gp_buffer *buffer)
{
        int8_t result = peek_int8(buffer);
        retrieve(buffer, sizeof(int8_t));
        
        return result;
}

char* read_str(gp_buffer *buffer, int32_t len)
{
        char *result = peek_str(buffer, len);
        retrieve(buffer, len);
        
        return result;
}

ProtobufCMessage* read_pb_msg(gp_buffer *buffer, void *desc, int32_t len)
{
        ProtobufCMessage *result = peek_pb_msg(buffer, desc, len);
        retrieve(buffer, len);
        
        return result;
}
