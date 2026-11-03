#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "contact.h"


/* Копирует строку с ограничением длины MAX_STR_LEN - 1 символов. */
static void safe_copy(char *dest, const char *src) {
    if (dest == NULL) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    size_t length = strlen(src);

    if (length >= MAX_STR_LEN) {
        length = MAX_STR_LEN - 1;
    }

    memmove(dest, src, length);
    dest[length] = '\0';
}


/* Инициализация телефонной книги. */
void init_phonebook(Phonebook *pb) {
    if (pb == NULL) {
        return;
    }

    pb->head = NULL;
    pb->tail = NULL;
    pb->count = 0;
}


/* Инициализация данных одного контакта. */
static void init_contact(Contact *c) {
    if (c != NULL) {
        memset(c, 0, sizeof(*c));
    }
}


/* Освобождение динамических массивов контакта. */
static void free_contact(Contact *c) {
    if (c == NULL) {
        return;
    }

    free(c->phones);
    free(c->emails);
    free(c->socials);
    free(c->messengers);

    c->phones = NULL;
    c->emails = NULL;
    c->socials = NULL;
    c->messengers = NULL;

    c->phone_count = 0;
    c->phone_capacity = 0;
    c->email_count = 0;
    c->email_capacity = 0;
    c->social_count = 0;
    c->social_capacity = 0;
    c->messenger_count = 0;
    c->messenger_capacity = 0;
}


/* Освобождение всей телефонной книги. */
void free_phonebook(Phonebook *pb) {
    if (pb == NULL) {
        return;
    }

    ContactNode *current = pb->head;

    while (current != NULL) {
        ContactNode *next_node = current->next;

        free_contact(&current->data);
        free(current);

        current = next_node;
    }

    pb->head = NULL;
    pb->tail = NULL;
    pb->count = 0;
}


/* Увеличение динамического массива строк. */
static String64 *expand_array(String64 *array, int *capacity) {
    if (capacity == NULL) {
        return array;
    }

    int new_capacity = (*capacity == 0) ? 2 : (*capacity * 2);

    String64 *new_array = realloc(
        array,
        (size_t)new_capacity * sizeof(*new_array)
    );

    if (new_array == NULL) {
        perror("Не удалось выделить память");
        exit(EXIT_FAILURE);
    }

    *capacity = new_capacity;
    return new_array;
}


/* Сравнение контактов: сначала фамилия, затем имя. */
static int compare_contacts(const char *last_name_1,
                            const char *first_name_1,
                            const char *last_name_2,
                            const char *first_name_2) {
    int result = strcmp(last_name_1, last_name_2);

    if (result == 0) {
        result = strcmp(first_name_1, first_name_2);
    }

    return result;
}


/* Вставка узла в список в отсортированное место. */
static void insert_sorted_node(Phonebook *pb, ContactNode *node) {
    if (pb == NULL || node == NULL) {
        return;
    }

    ContactNode *current = pb->head;

    /*
     * Идём до первого контакта, который больше нового.
     * При одинаковых фамилии и имени новый узел вставится после них.
     */
    while (current != NULL &&
           compare_contacts(current->data.last_name,
                            current->data.first_name,
                            node->data.last_name,
                            node->data.first_name) <= 0) {
        current = current->next;
    }

    if (current == NULL) {
        /* Вставка в конец списка или в пустой список. */
        node->prev = pb->tail;
        node->next = NULL;

        if (pb->tail != NULL) {
            pb->tail->next = node;
        } else {
            pb->head = node;
        }

        pb->tail = node;
    } else {
        /* Вставка перед current. */
        node->next = current;
        node->prev = current->prev;

        if (current->prev != NULL) {
            current->prev->next = node;
        } else {
            pb->head = node;
        }

        current->prev = node;
    }
}


/* Отсоединение узла от списка без освобождения памяти. */
static void unlink_node(Phonebook *pb, ContactNode *node) {
    if (pb == NULL || node == NULL) {
        return;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        pb->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        pb->tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}


/* Поиск узла по адресу хранящегося в нём контакта. */
static ContactNode *find_node_by_contact(Phonebook *pb, Contact *c) {
    if (pb == NULL || c == NULL) {
        return NULL;
    }

    ContactNode *node = pb->head;

    while (node != NULL) {
        if (&node->data == c) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}


/*
 * Добавление контакта.
 * Фамилия и имя передаются как обязательные аргументы.
 * Остальные поля передаются парами:
 * FieldType, строка, ..., FIELD_END.
 */
Contact *add_contact(Phonebook *pb,
                     const char *last_name,
                     const char *first_name,
                     ...) {
    if (pb == NULL ||
        last_name == NULL || last_name[0] == '\0' ||
        first_name == NULL || first_name[0] == '\0') {
        printf("Ошибка: фамилия и имя обязательны.\n");
        return NULL;
    }

    ContactNode *new_node = malloc(sizeof(*new_node));

    if (new_node == NULL) {
        perror("Не удалось выделить память");
        return NULL;
    }

    init_contact(&new_node->data);
    new_node->prev = NULL;
    new_node->next = NULL;

    safe_copy(new_node->data.last_name, last_name);
    safe_copy(new_node->data.first_name, first_name);

    va_list args;
    va_start(args, first_name);

    FieldType type;

    while ((type = (FieldType)va_arg(args, int)) != FIELD_END) {
        /*
         * В main.c дополнительные строки передаются как char-массивы,
         * поэтому здесь читается char *.
         */
        char *value = va_arg(args, char *);

        if (value == NULL || value[0] == '\0') {
            continue;
        }

        switch (type) {
            case FIELD_PATRONYMIC:
                safe_copy(new_node->data.patronymic, value);
                break;

            case FIELD_WORKPLACE:
                safe_copy(new_node->data.workplace, value);
                break;

            case FIELD_JOB_TITLE:
                safe_copy(new_node->data.job_title, value);
                break;

            case FIELD_PHONE:
            case FIELD_EMAIL:
            case FIELD_SOCIAL:
            case FIELD_MESSENGER:
                add_array_item(&new_node->data, type, value);
                break;

            default:
                break;
        }
    }

    va_end(args);

    /* Сразу помещаем новый контакт в отсортированное место. */
    insert_sorted_node(pb, new_node);
    pb->count++;

    printf("Контакт %s %s успешно добавлен!\n",
           new_node->data.last_name,
           new_node->data.first_name);

    return &new_node->data;
}


/* Получение контакта по его текущему индексу в списке. */
Contact *get_contact(const Phonebook *pb, int index) {
    if (pb == NULL || index < 0 || index >= pb->count) {
        return NULL;
    }

    ContactNode *current = pb->head;

    for (int i = 0; i < index && current != NULL; i++) {
        current = current->next;
    }

    if (current == NULL) {
        return NULL;
    }

    return &current->data;
}


/* Удаление контакта по его текущему индексу. */
void delete_contact(Phonebook *pb, int index) {
    if (pb == NULL || index < 0 || index >= pb->count) {
        printf("Ошибка: неверный индекс.\n");
        return;
    }

    ContactNode *current = pb->head;

    for (int i = 0; i < index && current != NULL; i++) {
        current = current->next;
    }

    if (current == NULL) {
        printf("Ошибка: контакт не найден.\n");
        return;
    }

    unlink_node(pb, current);
    free_contact(&current->data);
    free(current);

    pb->count--;

    printf("Контакт удален.\n");
}


/*
 * Редактирование фамилии, имени или другого одиночного поля.
 * Если изменяется фамилия или имя, узел перемещается,
 * чтобы список снова был отсортирован.
 */
int edit_single_field(Phonebook *pb,
                      Contact *c,
                      FieldType type,
                      const char *new_value) {
    if (c == NULL || new_value == NULL) {
        return 0;
    }

    if (type == FIELD_LAST_NAME || type == FIELD_FIRST_NAME) {
        /* Фамилию и имя нельзя очистить. */
        if (new_value[0] == '\0') {
            return 0;
        }

        ContactNode *node = find_node_by_contact(pb, c);

        if (node == NULL) {
            return 0;
        }

        if (type == FIELD_LAST_NAME) {
            safe_copy(c->last_name, new_value);
        } else {
            safe_copy(c->first_name, new_value);
        }

        /* Ключ сортировки изменился: перемещаем узел. */
        unlink_node(pb, node);
        insert_sorted_node(pb, node);

        return 1;
    }

    /* Изменение этих полей на порядок списка не влияет. */
    switch (type) {
        case FIELD_PATRONYMIC:
            safe_copy(c->patronymic, new_value);
            return 1;

        case FIELD_WORKPLACE:
            safe_copy(c->workplace, new_value);
            return 1;

        case FIELD_JOB_TITLE:
            safe_copy(c->job_title, new_value);
            return 1;

        default:
            return 0;
    }
}


/* Добавление телефона, почты, соцсети или мессенджера. */
void add_array_item(Contact *c, FieldType type, const char *value) {
    if (c == NULL || value == NULL || value[0] == '\0') {
        return;
    }

    switch (type) {
        case FIELD_PHONE:
            if (c->phone_count == c->phone_capacity) {
                c->phones = expand_array(c->phones, &c->phone_capacity);
            }

            safe_copy(c->phones[c->phone_count], value);
            c->phone_count++;
            break;

        case FIELD_EMAIL:
            if (c->email_count == c->email_capacity) {
                c->emails = expand_array(c->emails, &c->email_capacity);
            }

            safe_copy(c->emails[c->email_count], value);
            c->email_count++;
            break;

        case FIELD_SOCIAL:
            if (c->social_count == c->social_capacity) {
                c->socials = expand_array(c->socials, &c->social_capacity);
            }

            safe_copy(c->socials[c->social_count], value);
            c->social_count++;
            break;

        case FIELD_MESSENGER:
            if (c->messenger_count == c->messenger_capacity) {
                c->messengers = expand_array(
                    c->messengers,
                    &c->messenger_capacity
                );
            }

            safe_copy(c->messengers[c->messenger_count], value);
            c->messenger_count++;
            break;

        default:
            break;
    }
}


/* Изменение одного элемента массива телефона, почты и т. д. */
void edit_array_item(Contact *c,
                     FieldType type,
                     int item_index,
                     const char *new_value) {
    if (c == NULL || new_value == NULL || item_index < 0) {
        return;
    }

    switch (type) {
        case FIELD_PHONE:
            if (item_index < c->phone_count) {
                safe_copy(c->phones[item_index], new_value);
            }
            break;

        case FIELD_EMAIL:
            if (item_index < c->email_count) {
                safe_copy(c->emails[item_index], new_value);
            }
            break;

        case FIELD_SOCIAL:
            if (item_index < c->social_count) {
                safe_copy(c->socials[item_index], new_value);
            }
            break;

        case FIELD_MESSENGER:
            if (item_index < c->messenger_count) {
                safe_copy(c->messengers[item_index], new_value);
            }
            break;

        default:
            break;
    }
}


/* Удаление одного элемента массива телефона, почты и т. д. */
void delete_array_item(Contact *c, FieldType type, int item_index) {
    if (c == NULL || item_index < 0) {
        return;
    }

    switch (type) {
        case FIELD_PHONE:
            if (item_index < c->phone_count) {
                for (int i = item_index; i < c->phone_count - 1; i++) {
                    safe_copy(c->phones[i], c->phones[i + 1]);
                }

                c->phone_count--;
                c->phones[c->phone_count][0] = '\0';
            }
            break;

        case FIELD_EMAIL:
            if (item_index < c->email_count) {
                for (int i = item_index; i < c->email_count - 1; i++) {
                    safe_copy(c->emails[i], c->emails[i + 1]);
                }

                c->email_count--;
                c->emails[c->email_count][0] = '\0';
            }
            break;

        case FIELD_SOCIAL:
            if (item_index < c->social_count) {
                for (int i = item_index; i < c->social_count - 1; i++) {
                    safe_copy(c->socials[i], c->socials[i + 1]);
                }

                c->social_count--;
                c->socials[c->social_count][0] = '\0';
            }
            break;

        case FIELD_MESSENGER:
            if (item_index < c->messenger_count) {
                for (int i = item_index; i < c->messenger_count - 1; i++) {
                    safe_copy(c->messengers[i], c->messengers[i + 1]);
                }

                c->messenger_count--;
                c->messengers[c->messenger_count][0] = '\0';
            }
            break;

        default:
            break;
    }
}


/* Вывод полной информации о контакте. */
void print_contact(const Contact *c) {
    if (c == NULL) {
        return;
    }

    printf("\n=== %s %s %s ===\n",
           c->last_name,
           c->first_name,
           c->patronymic);

    if (c->workplace[0] != '\0') {
        printf("Место работы: %s\n", c->workplace);
    }

    if (c->job_title[0] != '\0') {
        printf("Должность: %s\n", c->job_title);
    }

    if (c->phone_count > 0 && c->phones != NULL) {
        printf("Телефоны:\n");

        for (int i = 0; i < c->phone_count; i++) {
            printf("  [%d] %s\n", i, c->phones[i]);
        }
    }

    if (c->email_count > 0 && c->emails != NULL) {
        printf("E-mail:\n");

        for (int i = 0; i < c->email_count; i++) {
            printf("  [%d] %s\n", i, c->emails[i]);
        }
    }

    if (c->social_count > 0 && c->socials != NULL) {
        printf("Соцсети:\n");

        for (int i = 0; i < c->social_count; i++) {
            printf("  [%d] %s\n", i, c->socials[i]);
        }
    }

    if (c->messenger_count > 0 && c->messengers != NULL) {
        printf("Мессенджеры:\n");

        for (int i = 0; i < c->messenger_count; i++) {
            printf("  [%d] %s\n", i, c->messengers[i]);
        }
    }

    printf("====================\n");
}


/* Вывод списка контактов с их текущими индексами. */
void print_phonebook(const Phonebook *pb) {
    if (pb == NULL || pb->count == 0 || pb->head == NULL) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }

    printf("\n--- СПИСОК КОНТАКТОВ ---\n");

    ContactNode *current = pb->head;
    int index = 0;

    while (current != NULL) {
        printf("[%d] %s %s\n",
               index,
               current->data.last_name,
               current->data.first_name);

        current = current->next;
        index++;
    }

    printf("------------------------\n");
}