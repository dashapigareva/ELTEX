#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/types.h>
#include <sys/stat.h>

#define SYMBOLIC_BUF_LEN  10
#define OCTAL_BUF_LEN      5
#define BITS_BUF_LEN      16

#define PERM_MASK  07777

void format_symbolic(mode_t mode, char *buf);
void format_octal(mode_t mode, char *buf);
void format_bits(mode_t mode, char *buf);
int parse_octal(const char *str, mode_t *mode);
int parse_symbolic(const char *str, mode_t *mode);
int parse_mode(const char *str, mode_t *mode);
int apply_chmod_command(const char *cmd, mode_t *mode);

#endif