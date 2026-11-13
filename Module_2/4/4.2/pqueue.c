#include "pqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pq_init(PriorityQueue *pq) {
    pq->head = NULL;
}

bool pq_push(PriorityQueue *pq, const char *msg, uint8_t priority) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        return false;
    }

    new_node->priority = priority;
    strncpy(new_node->message, msg, MSG_LEN - 1);
    new_node->message[MSG_LEN - 1] = '\0';
    new_node->next = NULL;
    if (pq->head == NULL || priority > pq->head->priority) {
        new_node->next = pq->head; // Новый элемент указывает на старую голову
        pq->head = new_node; // Новый элемент сам становится головой
        return true;
    }

    // Иначе идем по очереди. При равных приоритетах новый встанет после старого — соблюдается принцип FIFO
    Node *current = pq->head;
    while (current->next != NULL && current->next->priority >= priority) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;

    return true;
}

bool pq_pop_first(PriorityQueue *pq, char *out_msg, uint8_t *out_priority) {
    if (pq->head == NULL) {
        return false; // Очередь пуста, извлекать нечего
    }

    Node *temp = pq->head; // Запоминаем текущую голову

    // Копируем данные для возврата пользователю
    if (out_msg != NULL) {
        strncpy(out_msg, temp->message, MSG_LEN);
    }
    if (out_priority != NULL) {
        *out_priority = temp->priority;
    }

    // Сдвигаем голову на следующий элемент
    pq->head = pq->head->next;

    // Освобождаем память старой головы
    free(temp);
    return true;
}

bool pq_pop_exact(PriorityQueue *pq, uint8_t target_priority, char *out_msg, uint8_t *out_priority) {
    if (pq->head == NULL) {
        return false;
    }

    // Если нужный элемент оказался прямо в голове очереди
    if (pq->head->priority == target_priority) {
        return pq_pop_first(pq, out_msg, out_priority);
    }
    Node *prev = pq->head;
    Node *curr = pq->head->next;

    while (curr != NULL) {
        if (curr->priority == target_priority) {
            if (out_msg != NULL) strncpy(out_msg, curr->message, MSG_LEN);
            if (out_priority != NULL) *out_priority = curr->priority;

            // Вырезаем узел curr из цепочки: предыдущий теперь указывает на следующий за curr
            prev->next = curr->next;
            
            free(curr);
            return true;
        }
        if (curr->priority < target_priority) {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

bool pq_pop_min_prio(PriorityQueue *pq, uint8_t min_priority, char *out_msg, uint8_t *out_priority) {
    if (pq->head == NULL || pq->head->priority < min_priority) {
        return false;
    }
    Node *curr = pq->head;
    uint8_t closest_priority = curr->priority;

    while (curr != NULL && curr->priority >= min_priority) {
        closest_priority = curr->priority;
        curr = curr->next;
    }
    return pq_pop_exact(pq, closest_priority, out_msg, out_priority);
}

void pq_print(const PriorityQueue *pq) {
    if (pq->head == NULL) {
        printf("  [ Очередь пуста ]\n");
        return;
    }

    Node *curr = pq->head;
    printf("  ГОЛОВА -> ");
    while (curr != NULL) {
        printf("[Приоритет: %3u | '%s'] -> \n", curr->priority, curr->message);
        curr = curr->next;
    }
    printf("КОНЕЦ (NULL)\n");
}

void pq_clear(PriorityQueue *pq) {
    while (pq_pop_first(pq, NULL, NULL)) {
    }
}