#include <stdio.h>
#include <string.h>
#include <math.h>
#include "calc.h"

int main() {
    char input[256];

    printf("Калькулятор\n");
    printf("Поддерживает: +, -, *, /, скобки ( ).\n");
    printf("Для выхода введите 'exit'.\n\n");

    while (1) {
        printf("Введите выражение: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            printf("Выход из программы\n");
            break;
        }

        if (strlen(input) == 0) continue;

        const char* expr_ptr = input;
        ASTNode* root = parse_expression(&expr_ptr);

        if (root) {
            double result = evaluate(root);
            printf("Результат: %g\n", result);
            free_ast(root);
        } else {
            printf("Ошибка синтаксиса.\n");
        }
    }

    return 0;
}