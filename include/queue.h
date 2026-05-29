#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"

// Nodo de la lista enlazada para la cola de listos
typedef struct node {
    pcb_t *pcb;         // Puntero al bloque de control del proceso
    struct node *next;  // Puntero al siguiente nodo
} node_t;

// Estructura de control de la cola
typedef struct {
    node_t *head;       // Puntero al inicio (de donde saca la CPU)
    node_t *tail;       // Puntero al final (donde inserta el generador)
    int size;           // Contador de procesos actuales en la cola
} queue_t;

// Funciones para gestionar la cola de procesos
queue_t* queue_init();
void enqueue(queue_t *q, pcb_t *pcb);
pcb_t* dequeue(queue_t *q);
int is_empty(queue_t *q);
void queue_free(queue_t *q);

#endif // QUEUE_H
