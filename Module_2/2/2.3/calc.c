#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "calc.h"

double op_add(double a, double b) { return a + b; }
double op_sub(double a, double b) { return a - b; }
double op_mul(double a, double b) { return a * b; }
double op_div(double a, double b) { return (b == 0.0) ? NAN : (a / b); }
double op_pow(double a, double b) { return pow(a, b); } 

#define MAX_OPS 10
Operation op_registry[MAX_OPS];
int op_count = 0;

void register_op(char sym, BinaryOpFunc f) {
    if (op_count < MAX_OPS) {
        op_registry[op_count].symbol = sym;
        op_registry[op_count].func = f;
        op_count++;
    }
}

void init_operations() {
    op_count = 0;
    register_op('+', op_add);
    register_op('-', op_sub);
    register_op('*', op_mul);
    register_op('/', op_div);
    register_op('^', op_pow);
}
//поиск в словаре
BinaryOpFunc get_op_func(char symbol) {
    for (int i = 0; i < op_count; i++) {
        if (op_registry[i].symbol == symbol) {
            return op_registry[i].func;//возвращает адрес функции в памяти
        }
    }
    return NULL;
}

ASTNode* create_node(char symbol, double value, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->op_symbol = symbol;
    node->value = value;
    node->left = left;
    node->right = right;
    return node;
}

void skip_whitespace(const char** expr) {
    while (isspace(**expr)) (*expr)++;
}

ASTNode* parse_expr(const char** expr);

ASTNode* parse_factor(const char** expr) {
    skip_whitespace(expr);
    
    if (**expr == '+' || **expr == '-') {
        char op = **expr;
        (*expr)++; 
        ASTNode* child = parse_factor(expr); 
        if (!child) return NULL; 
        if (op == '-') {
            ASTNode* minus_one = create_node('N', -1.0, NULL, NULL);
            return create_node('*', 0, minus_one, child);
        }
        return child; 
    }
    if (**expr == '(') {
        (*expr)++; 
        
        ASTNode* node = parse_expr(expr);
        if (!node) return NULL; 

        skip_whitespace(expr);
        if (**expr == ')') {
            (*expr)++; 
            return node; 
        } else {
            free_ast(node); 
            return NULL;    
        }
    }
    
    char* end;
    double val = strtod(*expr, &end);
    if (*expr != end) {
        *expr = end;
        return create_node('N', val, NULL, NULL); 
    }
    
    return NULL; 
}

ASTNode* parse_power(const char** expr) {
    ASTNode* left = parse_factor(expr);
    if (!left) return NULL;

    skip_whitespace(expr);
    while (**expr == '^') {
        char op = **expr;
        (*expr)++;
        ASTNode* right = parse_factor(expr);
        
        if (!right) {
            free_ast(left); 
            return NULL;
        }
        left = create_node(op, 0, left, right);
        skip_whitespace(expr);
    }
    return left;
}

ASTNode* parse_term(const char** expr) {
    ASTNode* left = parse_power(expr);
    if (!left) return NULL; 

    skip_whitespace(expr);
    while (**expr == '*' || **expr == '/') {
        char op = **expr;
        (*expr)++;
        ASTNode* right = parse_power(expr);
        
        if (!right) { 
            free_ast(left); 
            return NULL;
        }
        left = create_node(op, 0, left, right);
        skip_whitespace(expr);
    }
    return left;
}

ASTNode* parse_expr(const char** expr) {
    ASTNode* left = parse_term(expr);
    if (!left) return NULL; 

    skip_whitespace(expr);
    while (**expr == '+' || **expr == '-') {
        char op = **expr;
        (*expr)++;
        ASTNode* right = parse_term(expr);
        
        if (!right) { 
            free_ast(left);
            return NULL;
        }
        left = create_node(op, 0, left, right);
        skip_whitespace(expr);
    }
    return left;
}

ASTNode* parse_expression(const char** expression) {
    return parse_expr(expression);
}

double evaluate(ASTNode* node) {
    if (!node) return 0.0;

    if (node->op_symbol == 'N') {
        return node->value;
    }

    double left_val = evaluate(node->left);
    double right_val = evaluate(node->right);

    BinaryOpFunc func = get_op_func(node->op_symbol);
    if (func != NULL) {
        return func(left_val, right_val);
    }

    return 0.0;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}