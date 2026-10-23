#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "calc.h"

ASTNode* create_node(NodeType type, double value, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value;
    node->left = left;
    node->right = right;
    return node;
}

void skip_whitespace(const char** expr) {
    while (isspace(**expr)) {
        (*expr)++;
    }
}

// Заголовки для рекурсии
ASTNode* parse_expr(const char** expr);
ASTNode* parse_term(const char** expr);
ASTNode* parse_factor(const char** expr);

// Парсинг факторов
ASTNode* parse_factor(const char** expr) {
    skip_whitespace(expr);
    if (**expr == '(') {
        (*expr)++;
        ASTNode* node = parse_expr(expr);
        skip_whitespace(expr);
        if (**expr == ')') {
            (*expr)++;
        } else{
            free_ast(node);
            return NULL;
        }
        return node;
    }
    
    // Если это число
    char* end;
    double val = strtod(*expr, &end);
    if (*expr != end) {
        *expr = end;
        return create_node(OP_NUM, val, NULL, NULL);
    }
    return NULL; // Ошибка парсинга
}

// Парсинг термов (умножение и деление - высокий приоритет)
ASTNode* parse_term(const char** expr) {
    ASTNode* left = parse_factor(expr);
    skip_whitespace(expr);

    while (**expr == '*' || **expr == '/') {
        char op = **expr;
        (*expr)++;
        ASTNode* right = parse_factor(expr);
        if (op == '*') {
            left = create_node(OP_MUL, 0, left, right);
        } else {
            left = create_node(OP_DIV, 0, left, right);
        }
        skip_whitespace(expr);
    }
    return left;
}

ASTNode* parse_expr(const char** expr) {
    ASTNode* left = parse_term(expr);
    skip_whitespace(expr);

    while (**expr == '+' || **expr == '-') {
        char op = **expr;
        (*expr)++;
        ASTNode* right = parse_term(expr);
        if (op == '+') {
            left = create_node(OP_ADD, 0, left, right);
        } else {
            left = create_node(OP_SUB, 0, left, right);
        }
        skip_whitespace(expr);
    }
    return left;
}

ASTNode* parse_expression(const char** expression) {
    return parse_expr(expression);
}
double evaluate(ASTNode* node) {
    if (!node) return 0.0;

    switch (node->type) {
        case OP_NUM: return node->value;
        case OP_ADD: return evaluate(node->left) + evaluate(node->right);
        case OP_SUB: return evaluate(node->left) - evaluate(node->right);
        case OP_MUL: return evaluate(node->left) * evaluate(node->right);
        case OP_DIV: {
            double right_val = evaluate(node->right);
            if (right_val == 0.0) {
                return NAN;
            }
            return evaluate(node->left) / right_val;
        }
    }
    return 0.0;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}