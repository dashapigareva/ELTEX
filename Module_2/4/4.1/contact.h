#ifndef CONTACT_H
#define CONTACT_H

#include <stdarg.h>

#define MAX_STR_LEN 64

typedef char String64[MAX_STR_LEN];

typedef enum {
    FIELD_END = 0,
    FIELD_LAST_NAME,
    FIELD_FIRST_NAME,
    FIELD_PATRONYMIC,
    FIELD_WORKPLACE,
    FIELD_JOB_TITLE,
    FIELD_PHONE,
    FIELD_EMAIL,
    FIELD_SOCIAL,
    FIELD_MESSENGER
} FieldType;

typedef struct {
    char last_name[MAX_STR_LEN];
    char first_name[MAX_STR_LEN];
    char patronymic[MAX_STR_LEN];
    char workplace[MAX_STR_LEN];
    char job_title[MAX_STR_LEN];
    String64 *phones;
    int phone_count;
    int phone_capacity;

    String64 *emails;
    int email_count;
    int email_capacity;

    String64 *socials;
    int social_count;
    int social_capacity;

    String64 *messengers;
    int messenger_count;
    int messenger_capacity;
} Contact;

typedef struct ContactNode {
    Contact data;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;
typedef struct {
    ContactNode *head;
    ContactNode *tail; 
    int count;
} Phonebook;

void init_phonebook(Phonebook *pb);
void free_phonebook(Phonebook *pb);

Contact* add_contact(Phonebook *pb, const char *last_name, const char *first_name, ...);
void delete_contact(Phonebook *pb, int index);

Contact* get_contact(const Phonebook *pb, int index);

int edit_single_field(Phonebook *pb,Contact *c,FieldType type,const char *new_value);
void add_array_item(Contact *c, FieldType type, const char *value);
void edit_array_item(Contact *c, FieldType type, int item_index, const char *new_value);
void delete_array_item(Contact *c, FieldType type, int item_index);

void print_contact(const Contact *c);
void print_phonebook(const Phonebook *pb);

#endif