#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pqueue.h"

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//проверяет, чтобы ввели от 0 до 255
uint8_t input_priority(const char *prompt) {
    int prio;
    while (1) {
        printf("%s (0-255): ", prompt);
        if (scanf("%d", &prio) == 1 && prio >= 0 && prio <= 255) {
            clear_input_buffer();
            return (uint8_t)prio;
        }
        printf("  [Ошибка] Введите целое число строго от 0 до 255!\n");
        clear_input_buffer();
    }
}

int main() {
    srand(time(NULL));

    PriorityQueue pq;
    pq_init(&pq);

    int choice;
    int auto_task_counter = 1; // Счетчик для красивых имен авто-задач
    char msg_buf[MSG_LEN];
    uint8_t prio_buf;

    while (1) {
        printf("\n================ МЕНЮ ОЧЕРЕДИ С ПРИОРИТЕТОМ ================\n");
        printf("1. Добавить задачу вручную\n");
        printf("2. Сгенерировать N случайных процессов\n");
        printf("3. Извлечь: ПЕРВЫЙ элемент в очереди\n");
        printf("4. Извлечь: с УКАЗАННЫМ приоритетом (== X)\n");
        printf("5. Извлечь: с приоритетом НЕ НИЖЕ заданного (>= X)\n");
        printf("6. Показать текущую очередь\n");
        printf("7. Очистить всю очередь\n");
        printf("0. Выход\n");
        printf("============================================================\n");
        printf("Выберите действие: ");

        if (scanf("%d", &choice) != 1) {
            printf("\n  [Ошибка] Введите номер пункта меню (число)!\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        printf("\n");

        switch (choice) {
            case 1: {
                // Ручное добавление
                printf("Введите название задачи/сообщения: ");
                fgets(msg_buf, MSG_LEN, stdin);
                msg_buf[strcspn(msg_buf, "\n")] = '\0';

                prio_buf = input_priority("Введите приоритет задачи");

                if (pq_push(&pq, msg_buf, prio_buf)) {
                    printf("  [ОК] Задача '%s' (приоритет %u) успешно добавлена!\n", msg_buf, prio_buf);
                } else {
                    printf("  [Ошибка] Не удалось выделить память.\n");
                }
                pq_print(&pq);
                break;
            }

            case 2: {
                // Генерация N случайных задач
                int n;
                printf("Сколько случайных процессов сгенерировать? ");
                if (scanf("%d", &n) != 1 || n <= 0) {
                    printf("  [Ошибка] Количество должно быть положительным числом!\n");
                    clear_input_buffer();
                    break;
                }
                clear_input_buffer();

                for (int i = 0; i < n; i++) {
                    sprintf(msg_buf, "Процесс_%d", auto_task_counter++);
                    prio_buf = rand() % 256;
                    pq_push(&pq, msg_buf, prio_buf);
                    printf("  [+] Сгенерирован: '%s' (Приоритет: %u)\n", msg_buf, prio_buf);
                }
                printf("\nОчередь после генерации:\n");
                pq_print(&pq);
                break;
            }

            case 3: {
                if (pq_pop_first(&pq, msg_buf, &prio_buf)) {
                    printf("  [ИЗВЛЕЧЕНО] Первый элемент: '%s' (Приоритет: %u)\n", msg_buf, prio_buf);
                } else {
                    printf("  [ОТКАЗ] Очередь пуста, извлекать нечего!\n");
                }
                pq_print(&pq);
                break;
            }

            case 4: {
                uint8_t target = input_priority("Введите ТОЧНЫЙ приоритет для поиска");
                if (pq_pop_exact(&pq, target, msg_buf, &prio_buf)) {
                    printf("  [ИЗВЛЕЧЕНО] Найден элемент: '%s' (Приоритет: %u)\n", msg_buf, prio_buf);
                } else {
                    printf("  [ОТКАЗ] Элементов с приоритетом %u в очереди нет!\n", target);
                }
                pq_print(&pq);
                break;
            }

            case 5: {
                uint8_t min_p = input_priority("Введите МИНИМАЛЬНЫЙ допустимый приоритет");
                if (pq_pop_min_prio(&pq, min_p, msg_buf, &prio_buf)) {
                    printf("  [ИЗВЛЕЧЕНО] Подходящий элемент: '%s' (Приоритет: %u >= %u)\n", 
                           msg_buf, prio_buf, min_p);
                } else {
                    printf("  [ОТКАЗ] В очереди нет задач с приоритетом %u или выше!\n", min_p);
                }
                pq_print(&pq);
                break;
            }

            case 6: {
                printf("Текущее состояние очереди:\n");
                pq_print(&pq);
                break;
            }

            case 7: {
                pq_clear(&pq);
                printf("  [ОК] Очередь полностью очищена.\n");
                pq_print(&pq);
                break;
            }

            case 0: {
                printf("Очистка памяти и завершение работы...\n");
                pq_clear(&pq);
                return 0;
            }

            default:
                printf("  [Ошибка] Неизвестный пункт меню. Выберите от 0 до 7.\n");
                break;
        }
    }

    return 0;
}