
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "file_info.h"

char file_type_char(mode_t mode)
{
    if (S_ISREG(mode))  return '-';
    if (S_ISDIR(mode))  return 'd';
    if (S_ISLNK(mode))  return 'l';
    if (S_ISCHR(mode))  return 'c';
    if (S_ISBLK(mode))  return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '?';
}

int get_file_info(const char *path, file_info_t *info)
{
    struct stat st;

    if (path == NULL || info == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (stat(path, &st) != 0) {
        return -1;
    }

    info->mode = st.st_mode;
    info->type = file_type_char(st.st_mode);
    return 0;
}