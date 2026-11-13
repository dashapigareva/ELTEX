#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdint.h>
#include <stdbool.h>
#define MSG_LEN 64

typedef struct Node {
    uint8_t priority;
    char message[MSG_LEN];
    struct Node *next;      
} Node;

typedef struct {
    Node *head;
} PriorityQueue;
void pq_init(PriorityQueue *pq);
bool pq_push(PriorityQueue *pq, const char *msg, uint8_t priority);
bool pq_pop_first(PriorityQueue *pq, char *out_msg, uint8_t *out_priority);
bool pq_pop_exact(PriorityQueue *pq, uint8_t target_priority, char *out_msg, uint8_t *out_priority);
bool pq_pop_min_prio(PriorityQueue *pq, uint8_t min_priority, char *out_msg, uint8_t *out_priority);
void pq_print(const PriorityQueue *pq);
void pq_clear(PriorityQueue *pq);

#endif