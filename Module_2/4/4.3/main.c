#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "contact.h"

static int read_string(char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    return 1;
}

//если строка пустая или состоит только из пробелов
static int is_blank(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

static int read_required_string(const char *prompt, char *buffer, int max_len) {
    while (1) {
        printf("%s", prompt);
        if (!read_string(buffer, max_len)) return 0;
        if (!is_blank(buffer)) return 1;
        printf("Ошибка: это поле обязательно, оно не может быть пустым.\n");
    }
}
static int read_int(const char *prompt, int *value) {
    char buf[32];
    printf("%s", prompt);
    if (!read_string(buf, sizeof(buf))) return -1;

    char *end;
    long v = strtol(buf, &end, 10);
    if (end == buf) return 0;
    while (isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;
    if (v < INT_MIN || v > INT_MAX) return 0;

    *value = (int)v;
    return 1;
}

static Contact *select_contact(const Phonebook *pb) {
    char last[MAX_STR_LEN], first[MAX_STR_LEN];

    if (!read_required_string("Фамилия: ", last, MAX_STR_LEN)) return NULL;
    if (!read_required_string("Имя: ", first, MAX_STR_LEN)) return NULL;
    int total = find_contacts(pb, last, first, NULL, 0);
    if (total == 0) {
        printf("Контакт \"%s %s\" не найден.\n", last, first);
        return NULL;
    }

    Contact **found = malloc((size_t)total * sizeof(Contact *));
    if (found == NULL) {
        fprintf(stderr, "Ошибка памяти!\n");
        exit(1);
    }
    find_contacts(pb, last, first, found, total);

    Contact *result = NULL;

    if (total == 1) {
        result = found[0];
    } else {
        printf("\nНайдено контактов с таким именем: %d\n", total);
        for (int i = 0; i < total; i++) {
            const Contact *c = found[i];
            printf("  [id=%d] %s %s %s", c->id, c->last_name, c->first_name, c->patronymic);
            if (c->workplace[0] != '\0') printf(" | работа: %s", c->workplace);
            if (c->phone_count > 0)      printf(" | тел.: %s", c->phones[0]);
            printf("\n");
        }

        int id;
        if (read_int("Введите id нужного контакта: ", &id) == 1) {
            for (int i = 0; i < total; i++) {
                if (found[i]->id == id) { result = found[i]; break; }
            }
        }
        if (result == NULL) printf("Контакт с таким id среди найденных нет.\n");
    }

    free(found);
    return result;
}

static void manage_array_field(Contact *c, FieldType type, const char *field_name) {
    printf("\n--- Управление: %s ---\n", field_name);
    printf("1. Добавить\n");
    printf("2. Изменить существующий\n");
    printf("3. Удалить\n");
    printf("0. Отмена\n");

    int choice;
    if (read_int("Выбор: ", &choice) != 1) return;

    if (choice == 1) {
        char buffer[MAX_STR_LEN];
        printf("Введите данные (%s): ", field_name);
        read_string(buffer, MAX_STR_LEN);
        if (is_blank(buffer)) {
            printf("Пустое значение не добавлено.\n");
            return;
        }
        add_array_item(c, type, buffer);
        printf("Успешно добавлено!\n");
    }
    else if (choice == 2 || choice == 3) {
        //Количество элементов берём прямо из контакта
        int count = 0;
        switch (type) {
            case FIELD_PHONE:     count = c->phone_count;     break;
            case FIELD_EMAIL:     count = c->email_count;     break;
            case FIELD_SOCIAL:    count = c->social_count;    break;
            case FIELD_MESSENGER: count = c->messenger_count; break;
            default: break;
        }
        if (count == 0) {
            printf("Список пуст, нечего изменять/удалять.\n");
            return;
        }

        char prompt[96];
        snprintf(prompt, sizeof(prompt), "Введите индекс (от 0 до %d): ", count - 1);
        int idx;
        if (read_int(prompt, &idx) != 1) { printf("Неверный ввод!\n"); return; }
        if (idx < 0 || idx >= count)     { printf("Неверный индекс!\n"); return; }

        if (choice == 2) {
            char buffer[MAX_STR_LEN];
            printf("Введите новые данные: ");
            read_string(buffer, MAX_STR_LEN);
            if (is_blank(buffer)) {
                printf("Пустое значение не допускается. Для удаления выберите пункт 3.\n");
                return;
            }
            edit_array_item(c, type, idx, buffer);
            printf("Успешно изменено!\n");
        } else {
            delete_array_item(c, type, idx);
            printf("Успешно удалено!\n");
        }
    }
}

static int change_name(Phonebook *pb, Contact **c, int change_last) {
    char new_value[MAX_STR_LEN];
    const char *prompt = change_last ? "Введите новую фамилию: " : "Введите новое имя: ";
    if (!read_required_string(prompt, new_value, MAX_STR_LEN)) return 1;
    char old_last[MAX_STR_LEN], old_first[MAX_STR_LEN];
    char new_last[MAX_STR_LEN], new_first[MAX_STR_LEN];
    memcpy(old_last,  (*c)->last_name,  MAX_STR_LEN);
    memcpy(old_first, (*c)->first_name, MAX_STR_LEN);
    int id = (*c)->id;

    if (change_last) {
        memcpy(new_last, new_value, MAX_STR_LEN);
        memcpy(new_first, old_first, MAX_STR_LEN);
    } else {
        memcpy(new_last, old_last, MAX_STR_LEN);
        memcpy(new_first, new_value, MAX_STR_LEN);
    }

    PbStatus st = rename_contact(pb, old_last, old_first, id, new_last, new_first);
    if (st == PB_ERR_INVALID) {
        printf("Фамилия и имя не могут быть пустыми!\n");
        return 1;
    }
    if (st == PB_ERR_NOT_FOUND) {
        printf("Ошибка: контакт не найден в дереве.\n");
        return 0;
    }

    *c = find_contact(pb, new_last, new_first, id);
    if (*c == NULL) {
        printf("Ошибка: контакт потерян после переименования.\n");
        return 0;
    }
    printf("Изменено. Контакт перемещён в дереве: %s %s.\n", new_last, new_first);
    return 1;
}

static void edit_contact_menu(Phonebook *pb, Contact *c) {
    int choice = -1;
    while (choice != 0) {
        print_contact(c);

        printf("\nЧто вы хотите изменить у %s %s?\n", c->last_name, c->first_name);
        printf("1. Фамилия (контакт сменит место в дереве)\n");
        printf("2. Имя (контакт сменит место в дереве)\n");
        printf("3. Отчество\n");
        printf("4. Место работы\n");
        printf("5. Должность\n");
        printf("6. Телефоны\n");
        printf("7. E-mail\n");
        printf("8. Соцсети\n");
        printf("9. Мессенджеры\n");
        printf("0. Закончить редактирование контакта\n");

        int r = read_int("Выбор: ", &choice);
        if (r < 0) return;
        if (r == 0) { choice = -1; continue; }

        char buffer[MAX_STR_LEN];
        switch (choice) {
            case 1:
                if (!change_name(pb, &c, 1)) return;
                break;
            case 2:
                if (!change_name(pb, &c, 0)) return;
                break;
            case 3:
                printf("Введите новое отчество (оставьте пустым для удаления): ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_PATRONYMIC, buffer);
                break;
            case 4:
                printf("Введите новое место работы (пусто - удалить): ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_WORKPLACE, buffer);
                break;
            case 5:
                printf("Введите новую должность (пусто - удалить): ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_JOB_TITLE, buffer);
                break;
            case 6: manage_array_field(c, FIELD_PHONE, "Телефоны"); break;
            case 7: manage_array_field(c, FIELD_EMAIL, "E-mail"); break;
            case 8: manage_array_field(c, FIELD_SOCIAL, "Соцсети"); break;
            case 9: manage_array_field(c, FIELD_MESSENGER, "Мессенджеры"); break;
            case 0: break;
            default: printf("Неверный выбор.\n"); break;
        }
    }
}

static void action_add(Phonebook *pb) {
    char last_name[MAX_STR_LEN], first_name[MAX_STR_LEN];
    char patronymic[MAX_STR_LEN], workplace[MAX_STR_LEN], phone[MAX_STR_LEN];

    if (!read_required_string("Введите фамилию (обязательно): ", last_name, MAX_STR_LEN)) return;
    if (!read_required_string("Введите имя (обязательно): ", first_name, MAX_STR_LEN)) return;

    printf("Введите отчество (или Enter чтобы пропустить): ");
    read_string(patronymic, MAX_STR_LEN);
    printf("Введите место работы (или Enter чтобы пропустить): ");
    read_string(workplace, MAX_STR_LEN);
    printf("Введите телефон (или Enter чтобы пропустить): ");
    read_string(phone, MAX_STR_LEN);

    PbStatus st = add_contact(pb, last_name, first_name,
                              FIELD_PATRONYMIC, patronymic,
                              FIELD_WORKPLACE, workplace,
                              FIELD_PHONE, phone,
                              FIELD_END);
    if (st == PB_OK) printf("Контакт %s %s добавлен.\n", last_name, first_name);
    else             printf("Ошибка: фамилия и имя обязательны.\n");
}

static void action_delete(Phonebook *pb) {
    Contact *c = select_contact(pb);
    if (c == NULL) return;

    char last[MAX_STR_LEN], first[MAX_STR_LEN];
    memcpy(last,  c->last_name,  MAX_STR_LEN);
    memcpy(first, c->first_name, MAX_STR_LEN);
    int id = c->id;

    if (delete_contact(pb, last, first, id) == PB_OK)
        printf("Контакт %s %s удалён.\n", last, first);
    else
        printf("Ошибка: контакт не найден.\n");
}

static void action_traversal(const Phonebook *pb) {
    printf("\n1. Прямой обход (корень, левое, правое)\n");
    printf("2. Симметричный обход (левое, корень, правое)\n");
    printf("3. Обратный обход (левое, правое, корень)\n");
    printf("0. Назад\n");

    int choice;
    if (read_int("Выбор: ", &choice) != 1) return;

    switch (choice) {
        case 1: print_traversal(pb, ORDER_PRE);  break;
        case 2: print_traversal(pb, ORDER_IN);   break;
        case 3: print_traversal(pb, ORDER_POST); break;
        default: break;
    }
}

static void action_stats(const Phonebook *pb) {
    printf("\nКонтактов в книге: %d\n", pb->count);
    printf("Высота дерева:     %d\n", tree_height(pb));
    printf("АВЛ-условие:       %s\n",
           is_balanced(pb) ? "выполнено (|h(left) - h(right)| <= 1 в каждом узле)"
                           : "НАРУШЕНО!");
}

static void action_load_demo(Phonebook *pb) {
    static const char *last[]  = { "Андреев", "Борисов", "Васильев", "Громов", "Дмитриев",
                                   "Егоров", "Жуков", "Иванов", "Иванов" };
    static const char *first[] = { "Сергей", "Анна", "Олег", "Павел", "Мария",
                                   "Иван", "Пётр", "Иван", "Иван" };
    static const char *phone[] = { "+7 900 000-00-01", "+7 900 000-00-02", "+7 900 000-00-03",
                                   "+7 900 000-00-04", "+7 900 000-00-05", "+7 900 000-00-06",
                                   "+7 900 000-00-07", "+7 900 000-00-08", "+7 900 000-00-09" };
    int n = (int)(sizeof(last) / sizeof(last[0]));
    for (int i = 0; i < n; i++)
        add_contact(pb, last[i], first[i], FIELD_PHONE, phone[i], FIELD_END);

    printf("Добавлено тестовых контактов: %d (два из них - тёзки Иванов Иван).\n", n);
}

int main(void) {
    Phonebook pb;
    init_phonebook(&pb);

    int choice = -1;
    while (choice != 0) {
        printf("\nГЛАВНОЕ МЕНЮ ТЕЛЕФОННОЙ КНИГИ (АВЛ-дерево)\n");
        printf("1. Добавить новый контакт\n");
        printf("2. Показать список контактов (по алфавиту)\n");
        printf("3. Показать детальную информацию о контакте\n");
        printf("4. Удалить контакт\n");
        printf("5. Редактировать данные контакта\n");
        printf("6. Показать дерево\n");
        printf("7. Обходы дерева (прямой, симметричный, обратный)\n");
        printf("8. Статистика и проверка баланса\n");
        printf("9. Загрузить тестовые данные\n");
        printf("0. Выход\n");

        int r = read_int("Ваш выбор: ", &choice);
        if (r < 0) { choice = 0; printf("\n"); break; }   /* EOF */
        if (r == 0) { choice = -1; printf("Введите число от 0 до 9.\n"); continue; }

        switch (choice) {
            case 1: action_add(&pb); break;
            case 2: print_phonebook(&pb); break;
            case 3: {
                Contact *c = select_contact(&pb);
                if (c != NULL) print_contact(c);
                break;
            }
            case 4: action_delete(&pb); break;
            case 5: {
                Contact *c = select_contact(&pb);
                if (c != NULL) edit_contact_menu(&pb, c);
                break;
            }
            case 6: print_tree(&pb); break;
            case 7: action_traversal(&pb); break;
            case 8: action_stats(&pb); break;
            case 9: action_load_demo(&pb); break;
            case 0: break;
            default: printf("Неверный выбор.\n"); break;
        }
    }

    free_phonebook(&pb);
    printf("Программа завершена.\n");
    return 0;
}