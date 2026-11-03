#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contact.h"

void safe_copy(char *dest, const char *src) {
    if (src != NULL) {
        strncpy(dest, src, MAX_STR_LEN - 1);
        dest[MAX_STR_LEN - 1] = '\0';
    } else {
        dest[0] = '\0';
    }
}

void init_phonebook(Phonebook *pb) {
    pb->head = NULL;
    pb->tail = NULL;
    pb->count = 0;
}

void init_contact(Contact *c) {
    memset(c, 0, sizeof(Contact));
}

void free_contact(Contact *c) {
    if (c->phones != NULL) free(c->phones);
    if (c->emails != NULL) free(c->emails);
    if (c->socials != NULL) free(c->socials);
    if (c->messengers != NULL) free(c->messengers);
}
void free_phonebook(Phonebook *pb) {
    ContactNode *current = pb->head;
    while (current != NULL) {
        ContactNode *next_node = current->next;
        free_contact(&current->data);
        free(current);                
        current = next_node;
    }
}

void* expand_array(void *array, int *capacity) {
    *capacity = (*capacity == 0) ? 2 : (*capacity * 2);
    void *new_array = realloc(array, (*capacity) * sizeof(String64));
    if (new_array == NULL) exit(1);
    return new_array;
}

int compare_contacts(const char *ln1, const char *fn1, const char *ln2, const char *fn2) {
    int res = strcmp(ln1, ln2); 
    if (res == 0) {
        res = strcmp(fn1, fn2);
    }
    return res;
}
Contact* add_contact(Phonebook *pb, const char *last_name, const char *first_name, ...) {
    ContactNode *new_node = (ContactNode*)malloc(sizeof(ContactNode));
    init_contact(&new_node->data);
    new_node->prev = NULL;
    new_node->next = NULL;

    safe_copy(new_node->data.last_name, last_name);
    safe_copy(new_node->data.first_name, first_name);

    va_list args;
    va_start(args, first_name);
    FieldType type;
    while ((type = va_arg(args, FieldType)) != FIELD_END) {
        const char *value = va_arg(args, const char *);
        switch (type) {
            case FIELD_PATRONYMIC: safe_copy(new_node->data.patronymic, value); break;
            case FIELD_WORKPLACE:  safe_copy(new_node->data.workplace, value); break;
            case FIELD_JOB_TITLE:  safe_copy(new_node->data.job_title, value); break;
            case FIELD_PHONE:
            case FIELD_EMAIL:
            case FIELD_SOCIAL:
            case FIELD_MESSENGER:
                add_array_item(&new_node->data, type, value);
                break;
            default: break;
        }
    }
    va_end(args);

    if (pb->head == NULL) {
        pb->head = new_node;
        pb->tail = new_node;
    } else {
        ContactNode *current = pb->head;
        while (current != NULL && compare_contacts(current->data.last_name, current->data.first_name, last_name, first_name) < 0) {
            current = current->next;
        }

        if (current == pb->head) {
            // Вставка в самое начало
            new_node->next = pb->head;
            pb->head->prev = new_node;
            pb->head = new_node;
        } else if (current == NULL) {
            // Вставка в самый конец
            new_node->prev = pb->tail;
            pb->tail->next = new_node;
            pb->tail = new_node;
        } else {
            // Вставка в середину перед current
            new_node->next = current;
            new_node->prev = current->prev;
            current->prev->next = new_node;
            current->prev = new_node;
        }
    }

    pb->count++;
    printf("Контакт %s %s успешно добавлен!\n", last_name, first_name);
    return &new_node->data;
}

Contact* get_contact(const Phonebook *pb, int index) {
    if (index < 0 || index >= pb->count) return NULL;
    ContactNode *current = pb->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return &current->data;
}

void delete_contact(Phonebook *pb, int index) {
    if (index < 0 || index >= pb->count) {
        printf("Ошибка: Неверный индекс.\n"); return;
    }
    
    ContactNode *current = pb->head;
    for (int i = 0; i < index; i++) current = current->next;

    if (current->prev != NULL) current->prev->next = current->next;
    else pb->head = current->next;

    if (current->next != NULL) current->next->prev = current->prev;
    else pb->tail = current->prev; 

    free_contact(&current->data);
    free(current);
    pb->count--;
    printf("Контакт удален.\n");
}

void edit_single_field(Contact *c, FieldType type, const char *new_value) {
    switch (type) {
        case FIELD_PATRONYMIC: safe_copy(c->patronymic, new_value); break;
        case FIELD_WORKPLACE:  safe_copy(c->workplace, new_value); break;
        case FIELD_JOB_TITLE:  safe_copy(c->job_title, new_value); break;
        default: break;
    }
}

void add_array_item(Contact *c, FieldType type, const char *value) {
    switch (type) {
        case FIELD_PHONE:
            if (c->phone_count == c->phone_capacity) c->phones = expand_array(c->phones, &c->phone_capacity);
            safe_copy(c->phones[c->phone_count++], value); break;
        case FIELD_EMAIL:
            if (c->email_count == c->email_capacity) c->emails = expand_array(c->emails, &c->email_capacity);
            safe_copy(c->emails[c->email_count++], value); break;
        case FIELD_SOCIAL:
            if (c->social_count == c->social_capacity) c->socials = expand_array(c->socials, &c->social_capacity);
            safe_copy(c->socials[c->social_count++], value); break;
        case FIELD_MESSENGER:
            if (c->messenger_count == c->messenger_capacity) c->messengers = expand_array(c->messengers, &c->messenger_capacity);
            safe_copy(c->messengers[c->messenger_count++], value); break;
        default: break;
    }
}

void edit_array_item(Contact *c, FieldType type, int item_index, const char *new_value) {
    switch (type) {
        case FIELD_PHONE: if (item_index >= 0 && item_index < c->phone_count) safe_copy(c->phones[item_index], new_value); break;
        case FIELD_EMAIL: if (item_index >= 0 && item_index < c->email_count) safe_copy(c->emails[item_index], new_value); break;
        case FIELD_SOCIAL: if (item_index >= 0 && item_index < c->social_count) safe_copy(c->socials[item_index], new_value); break;
        case FIELD_MESSENGER: if (item_index >= 0 && item_index < c->messenger_count) safe_copy(c->messengers[item_index], new_value); break;
        default: break;
    }
}

void delete_array_item(Contact *c, FieldType type, int item_index) {
    switch (type) {
        case FIELD_PHONE:
            if (item_index >= 0 && item_index < c->phone_count) {
                for (int i = item_index; i < c->phone_count - 1; i++) safe_copy(c->phones[i], c->phones[i + 1]);
                c->phone_count--;
            } break;
        case FIELD_EMAIL:
            if (item_index >= 0 && item_index < c->email_count) {
                for (int i = item_index; i < c->email_count - 1; i++) safe_copy(c->emails[i], c->emails[i + 1]);
                c->email_count--;
            } break;
        case FIELD_SOCIAL:
            if (item_index >= 0 && item_index < c->social_count) {
                for (int i = item_index; i < c->social_count - 1; i++) safe_copy(c->socials[i], c->socials[i + 1]);
                c->social_count--;
            } break;
        case FIELD_MESSENGER:
            if (item_index >= 0 && item_index < c->messenger_count) {
                for (int i = item_index; i < c->messenger_count - 1; i++) safe_copy(c->messengers[i], c->messengers[i + 1]);
                c->messenger_count--;
            } break;
        default: break;
    }
}

void print_contact(const Contact *c) {
    printf("\n=== %s %s %s ===\n", c->last_name, c->first_name, c->patronymic);
    if (strlen(c->workplace) > 0) printf("Место работы: %s\n", c->workplace);
    if (strlen(c->job_title) > 0) printf("Должность: %s\n", c->job_title);
    
    if (c->phone_count > 0) {
        printf("Телефоны:\n");
        for (int i = 0; i < c->phone_count; i++) printf("  [%d] %s\n", i, c->phones[i]);
    }
    if (c->email_count > 0) {
        printf("E-mail:\n");
        for (int i = 0; i < c->email_count; i++) printf("  [%d] %s\n", i, c->emails[i]);
    }
    if (c->social_count > 0) {
        printf("Соцсети:\n");
        for (int i = 0; i < c->social_count; i++) printf("  [%d] %s\n", i, c->socials[i]);
    }
    if (c->messenger_count > 0) {
        printf("Мессенджеры:\n");
        for (int i = 0; i < c->messenger_count; i++) printf("  [%d] %s\n", i, c->messengers[i]);
    }
    printf("====================\n");
}

void print_phonebook(const Phonebook *pb) {
    if (pb->count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    printf("\n--- СПИСОК КОНТАКТОВ ---\n");
    ContactNode *current = pb->head;
    int i = 0;
    while (current != NULL) {
        printf("[%d] %s %s\n", i, current->data.last_name, current->data.first_name);
        current = current->next;
        i++;
    }
    printf("------------------------\n");
}