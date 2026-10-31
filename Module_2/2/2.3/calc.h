#ifndef CALC_H
#define CALC_H

#include <math.h>

typedef double (*BinaryOpFunc)(double, double);

typedef struct {
    char symbol;
    BinaryOpFunc func;
} Operation;

typedef struct ASTNode {
    char op_symbol;// 'N' - если это число. Иначе '+', '-', '*', '/', '^'
    double value; 
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

ASTNode* parse_expression(const char** expression);
double evaluate(ASTNode* node);
void free_ast(ASTNode* node);

//регистрация базовых команд при запуске
void init_operations();

#endif