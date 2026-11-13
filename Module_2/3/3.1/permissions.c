#include <stdio.h>
#include <string.h>

#include "permissions.h"
//проверяет, является ли символ цифрой.
static int is_digit_char(char c)
{
    return c >= '0' && c <= '9';
}
// возвращает символ x/s/S/t/T/- для одной позиции.
static char exec_char(mode_t mode, mode_t x_bit, mode_t special_bit,
                      char lower, char upper)
{
    int has_x = (mode & x_bit) != 0;
    int has_special = (mode & special_bit) != 0;

    if (has_special) {
        return has_x ? lower : upper;
    }
    return has_x ? 'x' : '-';
}
// заполняет буквенное представление.
void format_symbolic(mode_t mode, char *buf)
{
    mode &= PERM_MASK;

    // владелец
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = exec_char(mode, S_IXUSR, S_ISUID, 's', 'S');

    // группа
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = exec_char(mode, S_IXGRP, S_ISGID, 's', 'S');

    //остальные 
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = exec_char(mode, S_IXOTH, S_ISVTX, 't', 'T');

    buf[9] = '\0';
}
//заполняет цифровое представление через snprintf.
void format_octal(mode_t mode, char *buf)
{
    snprintf(buf, OCTAL_BUF_LEN, "%04o", (unsigned int)(mode & PERM_MASK));
}
//заполняет битовое представление, группируя биты по три.
void format_bits(mode_t mode, char *buf)
{
    int pos = 0;

    mode &= PERM_MASK;
    for (int i = 11; i >= 0; i--) {
        buf[pos++] = ((mode >> i) & 1) ? '1' : '0';
        if (i % 3 == 0 && i != 0) {
            buf[pos++] = ' ';
        }
    }
    buf[pos] = '\0';
}
//разбирает три или четыре восьмеричные цифры.
int parse_octal(const char *str, mode_t *mode)
{
    size_t len;
    unsigned int value = 0;

    if (str == NULL || mode == NULL) {
        return -1;
    }

    len = strlen(str);
    if (len != 3 && len != 4) {
        return -1;
    }

    for (size_t i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '7') {
            return -1;
        }
        value = value * 8 + (unsigned int)(str[i] - '0');
    }

    *mode = (mode_t)value;
    return 0;
}
//разбирает девять символов rwx/s/t.
int parse_symbolic(const char *str, mode_t *mode)
{
    mode_t result = 0;

    if (str == NULL || mode == NULL) {
        return -1;
    }
    if (strlen(str) != 9) {
        return -1;
    }

    //владелец: rwx или s/S
    if (str[0] == 'r') result |= S_IRUSR;
    else if (str[0] != '-') return -1;

    if (str[1] == 'w') result |= S_IWUSR;
    else if (str[1] != '-') return -1;

    switch (str[2]) {
        case 'x': result |= S_IXUSR; break;
        case 's': result |= S_IXUSR | S_ISUID; break;
        case 'S': result |= S_ISUID; break;
        case '-': break;
        default:  return -1;
    }

    // группа: rwx или s/S
    if (str[3] == 'r') result |= S_IRGRP;
    else if (str[3] != '-') return -1;

    if (str[4] == 'w') result |= S_IWGRP;
    else if (str[4] != '-') return -1;

    switch (str[5]) {
        case 'x': result |= S_IXGRP; break;
        case 's': result |= S_IXGRP | S_ISGID; break;
        case 'S': result |= S_ISGID; break;
        case '-': break;
        default:  return -1;
    }

    if (str[6] == 'r') result |= S_IROTH;
    else if (str[6] != '-') return -1;

    if (str[7] == 'w') result |= S_IWOTH;
    else if (str[7] != '-') return -1;

    switch (str[8]) {
        case 'x': result |= S_IXOTH; break;
        case 't': result |= S_IXOTH | S_ISVTX; break;
        case 'T': result |= S_ISVTX; break;
        case '-': break;
        default:  return -1;
    }

    *mode = result;
    return 0;
}
//выбирает parse_octal или parse_symbolic по первому символу.
int parse_mode(const char *str, mode_t *mode)
{
    if (str == NULL || mode == NULL || str[0] == '\0') {
        return -1;
    }
    if (is_digit_char(str[0])) {
        return parse_octal(str, mode);
    }
    return parse_symbolic(str, mode);
}
//разбирает одну команду вида кто операция права и меняет маску.
static const char *apply_clause(const char *p, mode_t *result)
{
    int who_u = 0, who_g = 0, who_o = 0;
    int have_who = 0;
    mode_t affect;
    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
        if (*p == 'u') {
            who_u = 1;
        } else if (*p == 'g') {
            who_g = 1;
        } else if (*p == 'o') {
            who_o = 1;
        } else { /* 'a' */
            who_u = who_g = who_o = 1;
        }
        have_who = 1;
        p++;
    }
    if (!have_who) {
        who_u = who_g = who_o = 1;
    }
    if (*p != '+' && *p != '-' && *p != '=') {
        return NULL;
    }
    affect = 0;
    if (who_u) affect |= S_IRWXU | S_ISUID;
    if (who_g) affect |= S_IRWXG | S_ISGID;
    if (who_o) affect |= S_IRWXO | S_ISVTX;
    while (*p == '+' || *p == '-' || *p == '=') {
        char op = *p++;
        int r = 0, w = 0, x = 0, s = 0, t = 0;
        mode_t rwx, bits = 0;

        while (*p == 'r' || *p == 'w' || *p == 'x' || *p == 's' || *p == 't') {
            switch (*p) {
                case 'r': r = 1; break;
                case 'w': w = 1; break;
                case 'x': x = 1; break;
                case 's': s = 1; break;
                case 't': t = 1; break;
            }
            p++;
        }

        // Биты rwx для каждой выбранной группы
        rwx = (mode_t)((r ? 04 : 0) | (w ? 02 : 0) | (x ? 01 : 0));
        if (who_u) bits |= rwx << 6;
        if (who_g) bits |= rwx << 3;
        if (who_o) bits |= rwx << 0;
        if (s && who_u) bits |= S_ISUID;
        if (s && who_g) bits |= S_ISGID;
        if (t && who_o) bits |= S_ISVTX;

        switch (op) {
            case '+':
                *result |= bits;
                break;
            case '-':
                *result &= ~bits;
                break;
            case '=':
                *result = (*result & ~affect) | bits;
                break;
        }
    }

    return p;
}
//разбирает список команд через запятую, меняет маску только при полном успехе.
int apply_chmod_command(const char *cmd, mode_t *mode)
{
    mode_t result;
    const char *p;

    if (cmd == NULL || mode == NULL || cmd[0] == '\0') {
        return -1;
    }
    if (is_digit_char(cmd[0])) {
        mode_t value;
        if (parse_octal(cmd, &value) != 0) {
            return -1;
        }
        *mode = value;
        return 0;
    }
    result = *mode & PERM_MASK;
    p = cmd;

    while (1) {
        p = apply_clause(p, &result);
        if (p == NULL) {
            return -1;
        }
        if (*p == '\0') {
            break;
        }
        if (*p != ',') {
            return -1;
        }
        p++;
    }

    *mode = result;
    return 0;
}