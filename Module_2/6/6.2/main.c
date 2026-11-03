#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contact.h"
void read_string(char *buffer, int max_len) {
    memset(buffer, 0, max_len); 
    if (fgets(buffer, max_len, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; 
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int get_array_count(Contact *c, FieldType type) {
    switch (type) {
        case FIELD_PHONE: return c->phone_count;
        case FIELD_EMAIL: return c->email_count;
        case FIELD_SOCIAL: return c->social_count;
        case FIELD_MESSENGER: return c->messenger_count;
        default: return 0;
    }
}

void manage_array_field(Contact *c, FieldType type, const char *field_name) {
    int choice = -1;
    printf("\n--- Управление: %s ---\n", field_name);
    printf("1. Добавить\n");
    printf("2. Изменить существующий\n");
    printf("3. Удалить\n");
    printf("0. Отмена\n");
    printf("Выбор: ");
    
    if (scanf("%d", &choice) != 1) { clear_input_buffer(); return; }
    clear_input_buffer();

    int count = get_array_count(c, type);

    if (choice == 1) {
        char buffer[MAX_STR_LEN];
        printf("Введите данные (%s): ", field_name);
        read_string(buffer, MAX_STR_LEN);
        add_array_item(c, type, buffer);
        printf("Успешно добавлено!\n");
    } 
    else if (choice == 2 || choice == 3) {
        if (count == 0) {
            printf("Список пуст, нечего изменять/удалять.\n"); return;
        }
        printf("Введите индекс для изменения/удаления (от 0 до %d): ", count - 1);
        int idx;
        if (scanf("%d", &idx) != 1) { clear_input_buffer(); return; }
        clear_input_buffer();

        if (idx < 0 || idx >= count) {
            printf("Неверный индекс!\n"); return;
        }

        if (choice == 2) {
            char buffer[MAX_STR_LEN];
            printf("Введите новые данные: ");
            read_string(buffer, MAX_STR_LEN);
            edit_array_item(c, type, idx, buffer);
            printf("Успешно изменено!\n");
        } else if (choice == 3) {
            delete_array_item(c, type, idx);
            printf("Успешно удалено!\n");
        }
    }
}

void edit_contact_menu(Contact *c) {
    int choice = -1;
    while (choice != 0) {
        print_contact(c); 
        
        printf("\n--- Что вы хотите изменить у %s %s? ---\n", c->last_name, c->first_name);
        printf("1. Отчество\n");
        printf("2. Место работы\n");
        printf("3. Должность\n");
        printf("4. Телефоны\n");
        printf("5. E-mail\n");
        printf("6. Соцсети\n");
        printf("7. Мессенджеры\n");
        printf("0. Закончить редактирование контакта\n");
        printf("Выбор: ");

        if (scanf("%d", &choice) != 1) { clear_input_buffer(); continue; }
        clear_input_buffer();

        char buffer[MAX_STR_LEN];
        switch (choice) {
            case 1:
                printf("Введите новое отчество: ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_PATRONYMIC, buffer);
                break;
            case 2:
                printf("Введите новое место работы: ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_WORKPLACE, buffer);
                break;
            case 3:
                printf("Введите новую должность: ");
                read_string(buffer, MAX_STR_LEN);
                edit_single_field(c, FIELD_JOB_TITLE, buffer);
                break;
            case 4: manage_array_field(c, FIELD_PHONE, "Телефоны"); break;
            case 5: manage_array_field(c, FIELD_EMAIL, "E-mail"); break;
            case 6: manage_array_field(c, FIELD_SOCIAL, "Соцсети"); break;
            case 7: manage_array_field(c, FIELD_MESSENGER, "Мессенджеры"); break;
            case 0: break;
            default: printf("Неверный выбор.\n"); break;
        }
    }
}

int main() {
    Phonebook pb;
    init_phonebook(&pb);

    int choice = -1;
    while (choice != 0) {
        printf("\n=== ГЛАВНОЕ МЕНЮ ТЕЛЕФОННОЙ КНИГИ ===\n");
        printf("1. Добавить новый контакт\n");
        printf("2. Показать список контактов\n");
        printf("3. Показать детальную информацию о контакте\n");
        printf("4. Удалить контакт\n");
        printf("5. Редактировать данные контакта\n");
        printf("0. Выход\n");
        printf("Ваш выбор: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer(); continue;
        }
        clear_input_buffer();

        if (choice == 1) {
            char last_name[MAX_STR_LEN];
            char first_name[MAX_STR_LEN];

            printf("Введите фамилию (обязательно): ");
            read_string(last_name, MAX_STR_LEN);
            printf("Введите имя (обязательно): ");
            read_string(first_name, MAX_STR_LEN);
            Contact *new_contact = add_contact(&pb, last_name, first_name, FIELD_END);
            if (new_contact != NULL) {
                edit_contact_menu(new_contact);
            }
        } 
        else if (choice == 2) {
            print_phonebook(&pb);
        } 
        else if (choice == 3) {
            print_phonebook(&pb);
            if (pb.count == 0) continue;
            printf("Введите номер контакта для просмотра: ");
            int idx;
            if (scanf("%d", &idx) == 1) {
                Contact *c = get_contact(&pb, idx);
                if (c != NULL) print_contact(c);
                else printf("Неверный номер!\n");
            }
            clear_input_buffer();
        }
        else if (choice == 4) {
            print_phonebook(&pb);
            if (pb.count == 0) continue;
            printf("Введите номер контакта для удаления: ");
            int idx;
            if (scanf("%d", &idx) == 1) {
                delete_contact(&pb, idx);
            }
            clear_input_buffer();
        }
        else if (choice == 5) {
            print_phonebook(&pb);
            if (pb.count == 0) continue;
            printf("Введите номер контакта для редактирования: ");
            int idx;
            if (scanf("%d", &idx) == 1) {
                clear_input_buffer();
                Contact *c = get_contact(&pb, idx);
                if (c != NULL) edit_contact_menu(c);
                else printf("Неверный номер!\n");
            } else {
                clear_input_buffer();
            }
        }
    }

    free_phonebook(&pb);

    printf("Программа завершена.\n");
    return 0;
}