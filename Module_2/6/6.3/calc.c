#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "calc.h"


static OperationInfo* registry = NULL;
static int op_count = 0;
static int op_capacity = 0;

static int is_reserved_symbol(char c) {
    return c == '\0' || c == 'N' || c == '~' || c == '(' || c == ')' ||
           c == '.' || isdigit((unsigned char)c) || isspace((unsigned char)c);
}

const OperationInfo* find_operation(char symbol) {
    for (int i = 0; i < op_count; i++) {
        if (registry[i].symbol == symbol) {
            return &registry[i];
        }
    }
    return NULL;
}

int register_operation(const OperationInfo* info) {
    if (!info || !info->func || info->priority < 1) return -1;
    if (is_reserved_symbol(info->symbol) || find_operation(info->symbol)) return -1;

    if (op_count == op_capacity) {
        int new_cap = op_capacity ? op_capacity * 2 : 8;
        OperationInfo* tmp = realloc(registry, new_cap * sizeof(OperationInfo));
        if (!tmp) return -1;
        registry = tmp;
        op_capacity = new_cap;
    }
    registry[op_count++] = *info;//копируем описание из библиотеки
    return 0;
}

int get_operations_count(void) {
    return op_count;
}

const OperationInfo* get_operation_at(int index) {
    if (index < 0 || index >= op_count) return NULL;
    return &registry[index];
}

void clear_operations(void) {
    free(registry);
    registry = NULL;
    op_count = 0;
    op_capacity = 0;
}

static ASTNode* create_node(char symbol, double value, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->op_symbol = symbol;
    node->value = value;
    node->left = left;
    node->right = right;
    return node;
}
//парсер
static void skip_whitespace(const char** expr) {
    while (isspace((unsigned char)**expr)) (*expr)++;
}

static ASTNode* parse_binary(const char** expr, int min_priority);

static ASTNode* parse_factor(const char** expr) {
    skip_whitespace(expr);

    if (**expr == '+' || **expr == '-') {
        char sign = **expr;
        (*expr)++;
        ASTNode* child = parse_factor(expr);
        if (!child) return NULL;
        if (sign == '-') return create_node('~', 0, child, NULL);
        return child;
    }

    if (**expr == '(') {
        (*expr)++;
        ASTNode* node = parse_binary(expr, 1);
        if (!node) return NULL;
        skip_whitespace(expr);
        if (**expr == ')') {
            (*expr)++;
            return node;
        }
        free_ast(node);
        return NULL;
    }

    char* end;
    double val = strtod(*expr, &end);
    if (*expr != end) {
        *expr = end;
        return create_node('N', val, NULL, NULL);
    }
    return NULL;
}

//разбор бинарных операций
static ASTNode* parse_binary(const char** expr, int min_priority) {
    ASTNode* left = parse_factor(expr);
    if (!left) return NULL;

    while (1) {
        skip_whitespace(expr);
        const OperationInfo* op = find_operation(**expr);
        if (!op || op->priority < min_priority) break;

        (*expr)++;
        ASTNode* right = parse_binary(expr, op->priority + 1);
        if (!right) {
            free_ast(left);
            return NULL;
        }
        left = create_node(op->symbol, 0, left, right);
    }
    return left;
}

ASTNode* parse_expression(const char** expression) {
    ASTNode* root = parse_binary(expression, 1);
    if (!root) return NULL;

    skip_whitespace(expression);
    if (**expression != '\0') {
        free_ast(root);
        return NULL;
    }
    return root;
}
double evaluate(ASTNode* node) {
    if (!node) return 0.0;

    if (node->op_symbol == 'N') return node->value;

    double left_val = evaluate(node->left);
    if (node->op_symbol == '~') return -left_val;

    double right_val = evaluate(node->right);
    const OperationInfo* op = find_operation(node->op_symbol);
    if (op) return op->func(left_val, right_val);

    return NAN;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}