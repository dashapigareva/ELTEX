#include <stdio.h>
#include <string.h>
#include "calc.h"
#include "loader.h"

#define DEFAULT_PLUGIN_DIR "plugins"

int main(int argc, char* argv[]) {
    const char* plugin_dir = (argc > 1) ? argv[1] : DEFAULT_PLUGIN_DIR;
    char input[256];

    printf("Загрузка операций из каталога '%s':\n", plugin_dir);
    load_plugins(plugin_dir);

    if (get_operations_count() == 0) {
        printf("Не найдено ни одной операции. Проверьте каталог с библиотеками.\n");
        unload_plugins();
        return 1;
    }

    printf("\nКалькулятор\nДоступные операции: ");
    for (int i = 0; i < get_operations_count(); i++) {
        printf("%c ", get_operation_at(i)->symbol);
    }
    printf("\nДля выхода введите 'exit'.\n\n");

    while (1) {
        printf("Введите выражение: ");
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            printf("Выход из программы...\n");
            break;
        }
        if (input[0] == '\0') continue;

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

    unload_plugins();
    return 0;
}