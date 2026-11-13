#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_info.h"
#include "menu.h"
#include "permissions.h"

#define INPUT_LEN 256

static mode_t current_mode = 0;
static int have_mode = 0;
//чтение строки
static void read_line(char *buf, size_t size)
{
    size_t len;
    int c;
    fflush(stdout);

    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\nВвод завершен.\n");
        exit(EXIT_SUCCESS);
    }

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
}
//выводит маску буквами, цифрами и битами
static void print_three_formats(mode_t mode)
{
    char sym[SYMBOLIC_BUF_LEN];
    char oct[OCTAL_BUF_LEN];
    char bits[BITS_BUF_LEN];

    format_symbolic(mode, sym);
    format_octal(mode, oct);
    format_bits(mode, bits);

    printf("  Буквенное: %s\n", sym);
    printf("  Цифровое:  %s\n", oct);
    printf("  Битовое:   %s   (спец. | user | group | other)\n", bits);
}
//печатает главное меню
static void print_main_menu(void)
{
    printf("\n=== Маска прав доступа ===\n");
    printf("1. Ввести права\n");
    printf("2. Анализ прав файла\n");
    printf("3. Изменить права\n");
    printf("0. Выход\n");
    printf("Выбор: ");
}
//разбирает введённые права и запоминает их
static void action_input_mode(void)
{
    char input[INPUT_LEN];
    mode_t mode;

    printf("Введите права.\n");
    printf("  буквенно: rwsr-xr-t, rwxr-xr-x\n");
    printf("  цифрами:  755, 0644, 4755\n");
    printf("Права: ");
    read_line(input, sizeof input);

    if (input[0] == '\0') {
        printf("Ошибка: пустой ввод.\n");
        return;
    }

    if (parse_mode(input, &mode) != 0) {
        printf("Ошибка: неверный формат прав '%s'.\n", input);
        return;
    }

    current_mode = mode;
    have_mode = 1;

    printf("\nВведенные права:\n");
    print_three_formats(current_mode);
}
//получает права файла и выводит их
static void action_file(void)
{
    char path[INPUT_LEN];
    char sym[SYMBOLIC_BUF_LEN];
    file_info_t info;

    printf("Имя файла: ");
    read_line(path, sizeof path);

    if (path[0] == '\0') {
        printf("Ошибка: имя файла не задано.\n");
        return;
    }

    if (get_file_info(path, &info) != 0) {
        printf("Ошибка: %s: %s\n", path, strerror(errno));
        return;
    }
    current_mode = info.mode & PERM_MASK;
    have_mode = 1;

    format_symbolic(info.mode, sym);

    printf("\nФайл: %s\n", path);
    printf("  ls -l:     %c%s\n", info.type, sym);
    print_three_formats(current_mode);
    printf("Для сравнения выполните в терминале: ls -l \"%s\"\n", path);
}
//применяет команды chmod к текущей маске в памяти, файл не меняется
static void action_modify(void)
{
    char cmd[INPUT_LEN];
    mode_t new_mode;
    if (!have_mode) {
        printf("Права еще не заданы. Сначала введите их.\n");
        action_input_mode();
        if (!have_mode) {
            return;
        }
    }

    printf("\nТекущие права:\n");
    print_three_formats(current_mode);

    printf("\nКоманды:\n");
    printf("  цифрами:  755, 0644\n");
    printf("  символьно: u+x, go-w, a=r, u+rwx,g-x,o=, u+s, o+t\n");
    printf("Пустая строка - выход в меню.\n");

    while (1) {
        printf("chmod> ");
        read_line(cmd, sizeof cmd);

        if (cmd[0] == '\0') {
            break;
        }
        new_mode = current_mode;
        if (apply_chmod_command(cmd, &new_mode) != 0) {
            printf("Ошибка: неверная команда '%s'.\n", cmd);
            continue;
        }

        current_mode = new_mode;

        printf("\nПрава после команды '%s':\n", cmd);
        print_three_formats(current_mode);
    }

    printf("Изменения сохранены только в программе, файл не изменен.\n");
}
//показывает меню и вызывает нужный пункт
void menu_run(void)
{
    char choice[INPUT_LEN];

    while (1) {
        print_main_menu();
        read_line(choice, sizeof choice);

        if (strcmp(choice, "1") == 0) {
            action_input_mode();
        } else if (strcmp(choice, "2") == 0) {
            action_file();
        } else if (strcmp(choice, "3") == 0) {
            action_modify();
        } else if (strcmp(choice, "0") == 0) {
            printf("Выход из программы.\n");
            break;
        } else {
            printf("Неизвестный пункт меню: '%s'.\n", choice);
        }
    }
}