#include "gp.h"


size_t get_process_name(char **process_name)
{
    char process_dir[4096];
    int len = sizeof(process_dir);
    char* path_end;
    if(readlink("/proc/self/exe", process_dir,len) <=0)
        return -1;
    path_end = strrchr(process_dir, '/');
    if(path_end == NULL)
        return -1;
    ++path_end;

    size_t process_name_len = path_end - process_dir;
    *process_name = malloc(process_name_len);
    strcpy(*process_name, path_end);
    *path_end =' ';
    printf("process_name:%s\n", *process_name);

    return process_name_len;
}
