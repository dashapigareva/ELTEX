#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "contact.h"

static void safe_copy(char *dest, const char *src) {
    if (src != NULL) {
        strncpy(dest, src, MAX_STR_LEN - 1);
        dest[MAX_STR_LEN - 1] = '\0';
    } else {
        dest[0] = '\0';
    }
}
static int str_is_blank(const char *s) {
    if (s == NULL) return 1;
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

static void init_contact(Contact *c) {
    memset(c, 0, sizeof(Contact));
}

static void free_contact(Contact *c) {
    free(c->phones);
    free(c->emails);
    free(c->socials);
    free(c->messengers);
}

static void *expand_array(void *array, int *capacity) {
    *capacity = (*capacity == 0) ? 2 : (*capacity * 2);
    void *new_array = realloc(array, (size_t)(*capacity) * sizeof(String64));
    if (new_array == NULL) {
        fprintf(stderr, "Ошибка памяти!\n");
        exit(1);
    }
    return new_array;
}

static int compare_key(const char *last, const char *first, int id, const Contact *c) {
    int r = strcmp(last, c->last_name);
    if (r != 0) return r;
    r = strcmp(first, c->first_name);
    if (r != 0) return r;
    if (id < c->id) return -1;
    if (id > c->id) return 1;
    return 0;
}

static int compare_contacts(const Contact *a, const Contact *b) {
    return compare_key(a->last_name, a->first_name, a->id, b);
}

static int node_height(const TreeNode *n) {
    return (n == NULL) ? 0 : n->height;
}

static void update_height(TreeNode *n) {
    int hl = node_height(n->left);
    int hr = node_height(n->right);
    n->height = 1 + (hl > hr ? hl : hr);
}

static int balance_factor(const TreeNode *n) {
    return (n == NULL) ? 0 : node_height(n->left) - node_height(n->right);
}

static TreeNode *rotate_right(TreeNode *y) {
    TreeNode *x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static TreeNode *rotate_left(TreeNode *x) {
    TreeNode *y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

static TreeNode *rebalance(TreeNode *n) {
    update_height(n);
    int bf = balance_factor(n);

    if (bf > 1) {// левое поддерево выше
        if (balance_factor(n->left) < 0)
            n->left = rotate_left(n->left);
        return rotate_right(n);
    }
    if (bf < -1) {// правое поддерево выше
        if (balance_factor(n->right) > 0)
            n->right = rotate_right(n->right);
        return rotate_left(n);
    }
    return n;
}

static TreeNode *insert_node(TreeNode *root, TreeNode *node) {
    if (root == NULL) return node;
    if (compare_contacts(&node->data, &root->data) < 0)
        root->left = insert_node(root->left, node);
    else
        root->right = insert_node(root->right, node);
    return rebalance(root);
}

// Отсоединяет от дерева узел с наименьшим ключом и кладёт его в *min_out.Возвращает новый корень поддерева.
static TreeNode *detach_min(TreeNode *node, TreeNode **min_out) {
    if (node->left == NULL) {
        *min_out = node;
        return node->right;
    }
    node->left = detach_min(node->left, min_out);
    return rebalance(node);
}
static TreeNode *detach_node(TreeNode *node, const char *last, const char *first,
                             int id, TreeNode **out) {
    if (node == NULL) return NULL;

    int cmp = compare_key(last, first, id, &node->data);
    if (cmp < 0) {
        node->left = detach_node(node->left, last, first, id, out);
    } else if (cmp > 0) {
        node->right = detach_node(node->right, last, first, id, out);
    } else {
        *out = node;
        TreeNode *l = node->left;
        TreeNode *r = node->right;
        node->left = NULL;
        node->right = NULL;
        node->height = 1;

        if (l == NULL) return r;
        if (r == NULL) return l;

        TreeNode *m = NULL;
        r = detach_min(r, &m);
        m->left = l;
        m->right = r;
        return rebalance(m);
    }
    return rebalance(node);
}

void init_phonebook(Phonebook *pb) {
    pb->root = NULL;
    pb->count = 0;
    pb->next_id = 1;
}

static void free_tree(TreeNode *n) {
    if (n == NULL) return;
    free_tree(n->left);
    free_tree(n->right);
    free_contact(&n->data);
    free(n);
}

void free_phonebook(Phonebook *pb) {
    free_tree(pb->root);
    pb->root = NULL;
    pb->count = 0;
}

PbStatus add_contact(Phonebook *pb, const char *last_name, const char *first_name, ...) {
    if (str_is_blank(last_name) || str_is_blank(first_name))
        return PB_ERR_INVALID;      /* до va_start, поэтому это безопасно */

    TreeNode *node = calloc(1, sizeof(TreeNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка памяти!\n");
        exit(1);
    }
    init_contact(&node->data);
    node->height = 1;
    node->data.id = pb->next_id++;
    safe_copy(node->data.last_name, last_name);
    safe_copy(node->data.first_name, first_name);

    va_list args;
    va_start(args, first_name);
    int type;
    while ((type = va_arg(args, int)) != FIELD_END) {
        const char *value = va_arg(args, const char *);
        if (value == NULL || value[0] == '\0') continue;
        switch (type) {
            case FIELD_PATRONYMIC: safe_copy(node->data.patronymic, value); break;
            case FIELD_WORKPLACE:  safe_copy(node->data.workplace, value);  break;
            case FIELD_JOB_TITLE:  safe_copy(node->data.job_title, value);  break;
            case FIELD_PHONE:
            case FIELD_EMAIL:
            case FIELD_SOCIAL:
            case FIELD_MESSENGER:
                add_array_item(&node->data, (FieldType)type, value);
                break;
            default: break;
        }
    }
    va_end(args);

    pb->root = insert_node(pb->root, node);
    pb->count++;
    return PB_OK;
}

PbStatus delete_contact(Phonebook *pb, const char *last_name, const char *first_name, int id) {
    TreeNode *node = NULL;
    pb->root = detach_node(pb->root, last_name, first_name, id, &node);
    if (node == NULL) return PB_ERR_NOT_FOUND;

    free_contact(&node->data);
    free(node);
    pb->count--;
    return PB_OK;
}

PbStatus rename_contact(Phonebook *pb,
                        const char *old_last, const char *old_first, int id,
                        const char *new_last, const char *new_first) {
    if (str_is_blank(new_last) || str_is_blank(new_first))
        return PB_ERR_INVALID;

    // Копируем новые значения сразу
    char nl[MAX_STR_LEN], nf[MAX_STR_LEN];
    safe_copy(nl, new_last);
    safe_copy(nf, new_first);

    TreeNode *node = NULL;
    pb->root = detach_node(pb->root, old_last, old_first, id, &node);
    if (node == NULL) return PB_ERR_NOT_FOUND;

    // Меняем ключ вне дерева и вставляем узел на новое место
    safe_copy(node->data.last_name, nl);
    safe_copy(node->data.first_name, nf);
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    pb->root = insert_node(pb->root, node);
    return PB_OK;
}

static void collect(const TreeNode *n, const char *last, const char *first,
                    Contact **results, int max_results, int *total) {
    if (n == NULL) return;

    int cmp = strcmp(last, n->data.last_name);
    if (cmp == 0) cmp = strcmp(first, n->data.first_name);

    if (cmp < 0) {
        collect(n->left, last, first, results, max_results, total);
    } else if (cmp > 0) {
        collect(n->right, last, first, results, max_results, total);
    } else {
        collect(n->left, last, first, results, max_results, total);
        if (results != NULL && *total < max_results)
            results[*total] = (Contact *)&n->data;
        (*total)++;
        collect(n->right, last, first, results, max_results, total);
    }
}

int find_contacts(const Phonebook *pb, const char *last_name, const char *first_name,
                  Contact **results, int max_results) {
    int total = 0;
    collect(pb->root, last_name, first_name, results, max_results, &total);
    return total;
}

Contact *find_contact(const Phonebook *pb, const char *last_name, const char *first_name, int id) {
    const TreeNode *n = pb->root;
    while (n != NULL) {
        int cmp = compare_key(last_name, first_name, id, &n->data);
        if (cmp == 0) return (Contact *)&n->data;
        n = (cmp < 0) ? n->left : n->right;
    }
    return NULL;
}
static int get_array_ref(Contact *c, FieldType type,
                         String64 ***items, int **count, int **capacity) {
    switch (type) {
        case FIELD_PHONE:
            *items = &c->phones;     *count = &c->phone_count;     *capacity = &c->phone_capacity;
            return 1;
        case FIELD_EMAIL:
            *items = &c->emails;     *count = &c->email_count;     *capacity = &c->email_capacity;
            return 1;
        case FIELD_SOCIAL:
            *items = &c->socials;    *count = &c->social_count;    *capacity = &c->social_capacity;
            return 1;
        case FIELD_MESSENGER:
            *items = &c->messengers; *count = &c->messenger_count; *capacity = &c->messenger_capacity;
            return 1;
        default:
            return 0;
    }
}

void edit_single_field(Contact *c, FieldType type, const char *new_value) {
    switch (type) {
        case FIELD_PATRONYMIC: safe_copy(c->patronymic, new_value); break;
        case FIELD_WORKPLACE:  safe_copy(c->workplace, new_value);  break;
        case FIELD_JOB_TITLE:  safe_copy(c->job_title, new_value);  break;
        default: break;
    }
}

void add_array_item(Contact *c, FieldType type, const char *value) {
    String64 **items; int *count; int *capacity;
    if (!get_array_ref(c, type, &items, &count, &capacity)) return;

    if (*count == *capacity)
        *items = expand_array(*items, capacity);
    safe_copy((*items)[*count], value);
    (*count)++;
}

void edit_array_item(Contact *c, FieldType type, int item_index, const char *new_value) {
    String64 **items; int *count; int *capacity;
    if (!get_array_ref(c, type, &items, &count, &capacity)) return;

    if (item_index >= 0 && item_index < *count)
        safe_copy((*items)[item_index], new_value);
}

void delete_array_item(Contact *c, FieldType type, int item_index) {
    String64 **items; int *count; int *capacity;
    if (!get_array_ref(c, type, &items, &count, &capacity)) return;

    if (item_index >= 0 && item_index < *count) {
        for (int i = item_index; i < *count - 1; i++)
            safe_copy((*items)[i], (*items)[i + 1]);
        (*count)--;
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


static void pre_order(const TreeNode *n) {// корень, левое, правое
    if (n == NULL) return;
    printf("  %s %s\n", n->data.last_name, n->data.first_name);
    pre_order(n->left);
    pre_order(n->right);
}

static void in_order(const TreeNode *n) {// левое, корень, правое 
    if (n == NULL) return;
    in_order(n->left);
    printf("  %s %s\n", n->data.last_name, n->data.first_name);
    in_order(n->right);
}

static void post_order(const TreeNode *n) {// левое, правое, корень
    if (n == NULL) return;
    post_order(n->left);
    post_order(n->right);
    printf("  %s %s\n", n->data.last_name, n->data.first_name);
}

void print_phonebook(const Phonebook *pb) {
    if (pb->root == NULL) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    printf("\n--- СПИСОК КОНТАКТОВ (по алфавиту) ---\n");
    in_order(pb->root);
    printf("--------------------------------------\n");
}

void print_traversal(const Phonebook *pb, TraversalOrder order) {
    if (pb->root == NULL) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    switch (order) {
        case ORDER_PRE:
            printf("\n--- ПРЯМОЙ ОБХОД (корень, левое, правое) ---\n");
            pre_order(pb->root);
            break;
        case ORDER_IN:
            printf("\n--- СИММЕТРИЧНЫЙ ОБХОД (левое, корень, правое) ---\n");
            in_order(pb->root);
            break;
        case ORDER_POST:
            printf("\n--- ОБРАТНЫЙ ОБХОД (левое, правое, корень) ---\n");
            post_order(pb->root);
            break;
    }
    printf("--------------------------------------\n");
}

static void print_tree_rec(const TreeNode *n, int depth) {
    if (n == NULL) return;
    print_tree_rec(n->right, depth + 1);
    for (int i = 0; i < depth; i++) putchar('\t');
    printf("%s %s\n", n->data.last_name, n->data.first_name);
    print_tree_rec(n->left, depth + 1);
}

void print_tree(const Phonebook *pb) {
    if (pb->root == NULL) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    printf("\n--- ДЕРЕВО (корень слева, правая ветвь сверху) ---\n");
    print_tree_rec(pb->root, 0);
    printf("--------------------------------------------------\n");
}

static int check_balance(const TreeNode *n, int *ok) {
    if (n == NULL) return 0;
    int hl = check_balance(n->left, ok);
    int hr = check_balance(n->right, ok);
    int h = 1 + (hl > hr ? hl : hr);

    int diff = hl - hr;
    if (diff > 1 || diff < -1) *ok = 0;
    if (n->height != h) *ok = 0;
    return h;
}

int tree_height(const Phonebook *pb) {
    return node_height(pb->root);
}

int is_balanced(const Phonebook *pb) {
    int ok = 1;
    check_balance(pb->root, &ok);
    return ok;
}