#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    mode_t mode;
    char   type;
} file_info_t;

int get_file_info(const char *path, file_info_t *info);

char file_type_char(mode_t mode);

#endif