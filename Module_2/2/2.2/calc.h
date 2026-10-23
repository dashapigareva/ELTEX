#ifndef CALC_H
#define CALC_H
#include <math.h>
typedef enum {
    OP_NUM,//число
    OP_ADD,//сложение
    OP_SUB,//вычитание
    OP_MUL,//умножение
    OP_DIV// деление
} NodeType;
typedef struct ASTNode {
    NodeType type;
    double value;
    struct ASTNode* left; 
    struct ASTNode* right; 
} ASTNode;
ASTNode* parse_expression(const char** expression);
double evaluate(ASTNode* node);
void free_ast(ASTNode* node);

#endif