#ifndef CALC_H
#define CALC_H

#include "plugin_api.h"

typedef struct ASTNode {
    char op_symbol;   /* 'N' - число, '~' - унарный минус, иначе символ бинарной операции */
    double value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

int register_operation(const OperationInfo* info);
const OperationInfo* find_operation(char symbol);
int get_operations_count(void);
const OperationInfo* get_operation_at(int index);
void clear_operations(void);
ASTNode* parse_expression(const char** expression);
double evaluate(ASTNode* node);
void free_ast(ASTNode* node);

#endif