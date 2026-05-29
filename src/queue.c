#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "sync.h"

// Inicializa las variables globales de sincronización que declaramos en sync.h
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
int simulation_active = 1;

// Asigna memoria para la estructura de la cola
queue_t* queue_init() {
    queue_t *q = (queue_t*)malloc(sizeof(queue_t));
    if (!q) {
        perror("Error al asignar memoria para la cola");
        exit(EXIT_FAILURE);
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

// Inserta un proceso al final de la cola (Operación Segura) [cite: 34]
void enqueue(queue_t *q, pcb_t *pcb) {
    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    if (!new_node) {
        perror("Error al asignar memoria para el nodo");
        exit(EXIT_FAILURE);
    }
    new_node->pcb = pcb;
    new_node->next = NULL;

    // --- SECCIÓN CRÍTICA ---
    pthread_mutex_lock(&queue_mutex);

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->size++;

    // Imprimimos un log limpio para que el ingeniero vea el comportamiento en tiempo real
    printf("[COLA] Proceso PID %d insertado (Burst: %dms, Prioridad: %d). Total en cola: %d\n",
           pcb->pid, pcb->burst_time, pcb->priority, q->size);

    // Despierta a una CPU que esté esperando un proceso [cite: 36, 38]
    pthread_cond_signal(&queue_cond);

    pthread_mutex_unlock(&queue_mutex);
    // --- FIN SECCIÓN CRÍTICA ---
}

// Extrae el primer proceso de la cola (Operación Segura y Bloqueante) [cite: 38]
pcb_t* dequeue(queue_t *q) {
    // --- SECCIÓN CRÍTICA ---
    pthread_mutex_lock(&queue_mutex);

    // Si la cola está vacía pero la simulación sigue viva, la CPU espera pasivamente
    while (q->head == NULL && simulation_active) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    // Si la simulación terminó y no hay más procesos, retornamos NULL para apagar la CPU
    if (q->head == NULL && !simulation_active) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }

    node_t *temp = q->head;
    pcb_t *pcb = temp->pcb;

    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->size--;

    free(temp);
    pthread_mutex_unlock(&queue_mutex);
    // --- FIN SECCIÓN CRÍTICA ---

    return pcb;
}

// Verifica si la cola está vacía de forma segura
int is_empty(queue_t *q) {
    pthread_mutex_lock(&queue_mutex);
    int empty = (q->head == NULL);
    pthread_mutex_unlock(&queue_mutex);
    return empty;
}

// Libera la memoria remanente de la cola al apagar el sistema
void queue_free(queue_t *q) {
    if (!q) return;
    node_t *current = q->head;
    while (current != NULL) {
        node_t *next = current->next;
        free(current->pcb); // Libera el PCB guardado
        free(current);      // Libera el nodo
        current = next;
    }
    free(q);
}
