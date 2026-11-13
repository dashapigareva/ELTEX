#ifndef CONTACT_H
#define CONTACT_H

#include <stdarg.h>

#define MAX_STR_LEN 64
typedef char String64[MAX_STR_LEN];

typedef enum {
    FIELD_END = 0,
    FIELD_PATRONYMIC,
    FIELD_WORKPLACE,
    FIELD_JOB_TITLE,
    FIELD_PHONE,
    FIELD_EMAIL,
    FIELD_SOCIAL,
    FIELD_MESSENGER
} FieldType;

typedef enum {
    PB_OK = 0,
    PB_ERR_INVALID,
    PB_ERR_NOT_FOUND 
} PbStatus;

typedef enum {
    ORDER_PRE,
    ORDER_IN,
    ORDER_POST
} TraversalOrder;

typedef struct {
    int id;

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

typedef struct TreeNode {
    Contact data;
    struct TreeNode *left;
    struct TreeNode *right;
    int height;
} TreeNode;

typedef struct {
    TreeNode *root;
    int count;
    int next_id;
} Phonebook;

void init_phonebook(Phonebook *pb);
void free_phonebook(Phonebook *pb);
PbStatus add_contact(Phonebook *pb, const char *last_name, const char *first_name, ...);
PbStatus delete_contact(Phonebook *pb, const char *last_name, const char *first_name, int id);

PbStatus rename_contact(Phonebook *pb,
                        const char *old_last, const char *old_first, int id,
                        const char *new_last, const char *new_first);

int find_contacts(const Phonebook *pb, const char *last_name, const char *first_name,
                  Contact **results, int max_results);

Contact *find_contact(const Phonebook *pb, const char *last_name, const char *first_name, int id);

void edit_single_field(Contact *c, FieldType type, const char *new_value);
void add_array_item(Contact *c, FieldType type, const char *value);
void edit_array_item(Contact *c, FieldType type, int item_index, const char *new_value);
void delete_array_item(Contact *c, FieldType type, int item_index);

void print_contact(const Contact *c);
void print_phonebook(const Phonebook *pb);
void print_traversal(const Phonebook *pb, TraversalOrder order);
void print_tree(const Phonebook *pb); 

int tree_height(const Phonebook *pb);
int is_balanced(const Phonebook *pb);

#endif