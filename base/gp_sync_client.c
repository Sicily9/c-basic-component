#include "gp.h"

static dict * sync_client_pb_map = NULL;

static dict* get_sync_client_pb_map(void)
{
    if(sync_client_pb_map == NULL){
        sync_client_pb_map = dictCreate(STRING_DICT, 0);
    }

    return sync_client_pb_map;
}

void register_name_sync_client_pb_map(char *name, const ProtobufCMessageDescriptor *desc)
{
    dict *map = get_sync_client_pb_map();
    dictAdd(map, name, (void *)desc);
}

static int32_t read_pb_header(int32_t fd, char **name, int *msg_len)
{
    int n, magic, len, name_len;
    n = read(fd, (char *)&magic, 4);
    if(n < 0){
        perror("read magic fail\n");
        close(fd);
        return -1;
    }

    if(ntohl(magic) != 0x1343EA4){
        perror("magic error\n");
        close(fd);
        return -1;
    }

    n = read(fd, (char *)&len, 4);
    if(n < 0){
        perror("read len fail\n");
        close(fd);
        return -1;
    }
    len = ntohl(len);

    n = read(fd, (char *)&name_len, 4);
    if(n < 0){
        perror("read name_len fail\n");
        close(fd);
        return -1;
    }
    name_len = ntohl(name_len);
    
    *name = malloc(name_len);
    n = read(fd, *name, name_len);
    if(n < 0){
        perror("read name fail\n");
        close(fd);
        return -1;
    }

    *msg_len = len - 4 - name_len;
    return 0;
}

static int32_t send_all(int *fd, char *ipc_server, char *msg, int32_t len)
{
    *fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un serun;
    memset(&serun, 0, sizeof(struct sockaddr_un));
    serun.sun_family = AF_UNIX;

    char path[40] = {0};
    snprintf(path, 40, "/tmp/%s.socket", ipc_server);
    strncpy(serun.sun_path, path, 40);
    
    int address_len = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);
    connect(*fd, (struct sockaddr *)&serun, address_len);
    
    int n = send_all_sync(*fd, msg, &len);
    if(-1 == n){
        perror("send fail");
        close(*fd);
        return -1;
    }

    return 0;
}

int32_t send_msg(char *ipc_server, char *msg, int32_t len)
{
    int fd = 0;
    int n = send_all(&fd, ipc_server, msg, len);
    if(-1 == n){
        perror("send fail");
        return -1;
    }
    close(fd);
    return 0;
}

ProtobufCMessage* send_msg_and_recv(char *ipc_server, char *msg, int32_t len)
{
    int fd = 0;
    int n = send_all(&fd, ipc_server, msg, len);
    if(-1 == n){
        return NULL;
    }
    
    char *name;
    int msg_len;
    n = read_pb_header(fd, &name, &msg_len);
    if(-1 == n){
        return NULL;
    }
    
    ProtobufCMessageDescriptor *desc = dictFetchValue(get_sync_client_pb_map(), name);
    if(unlikely(desc == NULL)){
        printf("unknown msg:%s\n", name);
        close(fd);
        return NULL;
    }
    free(name);
    
    char *msg_buf = malloc(msg_len);
    n = read_all_sync(fd, msg_buf, &msg_len);
    if(n < 0){
        perror("read name fail\n");
        close(fd);
        return NULL;
    }
    
    ProtobufCMessage* pb_msg = protobuf_c_message_unpack(desc, NULL, msg_len, (const uint8_t *)msg_buf);
    free(msg_buf);
    close(fd);

    return pb_msg;
}

