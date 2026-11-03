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
    pb->contacts = NULL; 
    pb->count = 0;
    pb->capacity = 0;
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
    for (int i = 0; i < pb->count; i++) {
        free_contact(&pb->contacts[i]);
    }
    if (pb->contacts != NULL) {
        free(pb->contacts);
    }
}

void* expand_array(void *array, int *capacity) {
    *capacity = (*capacity == 0) ? 2 : (*capacity * 2);
    void *new_array = realloc(array, (*capacity) * sizeof(String64));
    if (new_array == NULL) {
        exit(1);
    }
    return new_array;
}

void add_contact(Phonebook *pb, const char *last_name, const char *first_name, ...) {
    if (last_name == NULL || first_name == NULL ||
        strlen(last_name) == 0 || strlen(first_name) == 0) {
        printf("Ошибка: фамилия и имя обязательны для заполнения.\n");
        return;
    }
    if (pb->count == pb->capacity) {
        pb->capacity = (pb->capacity == 0) ? 2 : (pb->capacity * 2);
        pb->contacts = realloc(pb->contacts, pb->capacity * sizeof(Contact));
        if (pb->contacts == NULL) {
            printf("ошибка памяти!\n");
            exit(1);
        }
    }

    Contact *new_contact = &pb->contacts[pb->count];
    init_contact(new_contact);

    safe_copy(new_contact->last_name, last_name);
    safe_copy(new_contact->first_name, first_name);

    va_list args;//указатель, который двигается по стопке
    va_start(args, first_name);
    FieldType type;
    while ((type = va_arg(args, FieldType)) != FIELD_END) {
        const char *value = va_arg(args, const char *);
        if (strlen(value) == 0) continue;
        switch (type) {
            case FIELD_PATRONYMIC: safe_copy(new_contact->patronymic, value); break;
            case FIELD_WORKPLACE:  safe_copy(new_contact->workplace, value); break;
            case FIELD_JOB_TITLE:  safe_copy(new_contact->job_title, value); break;
            case FIELD_PHONE:
            case FIELD_EMAIL:
            case FIELD_SOCIAL:
            case FIELD_MESSENGER:
                add_array_item(new_contact, type, value);
                break;
            default: break;
        }
    }
    va_end(args);

    pb->count++;
    printf("Контакт %s %s успешно добавлен!\n", last_name, first_name);
}

void delete_contact(Phonebook *pb, int index) {
    if (index < 0 || index >= pb->count) {
        printf("Ошибка: Неверный индекс.\n"); return;
    }
    free_contact(&pb->contacts[index]); 

    for (int i = index; i < pb->count - 1; i++) {
        pb->contacts[i] = pb->contacts[i + 1];
    }
    pb->count--;
    printf("Контакт удален.\n");
}

void edit_single_field(Contact *c, FieldType type, const char *new_value) {
    switch (type) {
        case FIELD_LAST_NAME:
            if (new_value != NULL && strlen(new_value) > 0)
                safe_copy(c->last_name, new_value);
            break;
        case FIELD_FIRST_NAME:
            if (new_value != NULL && strlen(new_value) > 0)
                safe_copy(c->first_name, new_value);
            break;
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
            safe_copy(c->phones[c->phone_count++], value);
            break;
        case FIELD_EMAIL:
            if (c->email_count == c->email_capacity) c->emails = expand_array(c->emails, &c->email_capacity);
            safe_copy(c->emails[c->email_count++], value);
            break;
        case FIELD_SOCIAL:
            if (c->social_count == c->social_capacity) c->socials = expand_array(c->socials, &c->social_capacity);
            safe_copy(c->socials[c->social_count++], value);
            break;
        case FIELD_MESSENGER:
            if (c->messenger_count == c->messenger_capacity) c->messengers = expand_array(c->messengers, &c->messenger_capacity);
            safe_copy(c->messengers[c->messenger_count++], value);
            break;
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
            }
            break;
        case FIELD_EMAIL:
            if (item_index >= 0 && item_index < c->email_count) {
                for (int i = item_index; i < c->email_count - 1; i++) safe_copy(c->emails[i], c->emails[i + 1]);
                c->email_count--;
            }
            break;
        case FIELD_SOCIAL:
            if (item_index >= 0 && item_index < c->social_count) {
                for (int i = item_index; i < c->social_count - 1; i++) safe_copy(c->socials[i], c->socials[i + 1]);
                c->social_count--;
            }
            break;
        case FIELD_MESSENGER:
            if (item_index >= 0 && item_index < c->messenger_count) {
                for (int i = item_index; i < c->messenger_count - 1; i++) safe_copy(c->messengers[i], c->messengers[i + 1]);
                c->messenger_count--;
            }
            break;
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
    for (int i = 0; i < pb->count; i++) {
        printf("[%d] %s %s\n", i, pb->contacts[i].last_name, pb->contacts[i].first_name);
    }
    printf("------------------------\n");
}