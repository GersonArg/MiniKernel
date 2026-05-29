#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "metrics.h"
#include "sync.h"

// Variables globales del sistema
queue_t *ready_queue = NULL;
static int global_simulation_time = 0;
static pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

// Almacenamiento seguro para procesos terminados
static pcb_t **finished_processes = NULL;
static int finished_count = 0;
static int finished_capacity = 10;
static pthread_mutex_t metrics_mutex = PTHREAD_MUTEX_INITIALIZER;

// Inicializa el tiempo global
void init_simulation_time() {
    pthread_mutex_lock(&time_mutex);
    global_simulation_time = 0;
    pthread_mutex_unlock(&time_mutex);
}

// Incrementa de forma segura el reloj global
void increment_simulation_time(int amount) {
    pthread_mutex_lock(&time_mutex);
    global_simulation_time += amount;
    pthread_mutex_unlock(&time_mutex);
}

// Obtiene de forma segura el tiempo actual
int get_current_simulation_time() {
    pthread_mutex_lock(&time_mutex);
    int current_time = global_simulation_time;
    pthread_mutex_unlock(&time_mutex);
    return current_time;
}

// Inicializa el arreglo donde guardaremos las métricas finales
void metrics_init() {
    finished_processes = (pcb_t**)malloc(finished_capacity * sizeof(pcb_t*));
    if (!finished_processes) {
        perror("Error al inicializar almacenamiento de métricas");
        exit(EXIT_FAILURE);
    }
}

// Guarda de forma segura un PCB terminado para procesarlo al final
void register_finished_process(pcb_t *pcb) {
    pthread_mutex_lock(&metrics_mutex);

    // Si el arreglo se llena, duplicamos su capacidad (Estructura dinámica limpia)
    if (finished_count >= finished_capacity) {
        finished_capacity *= 2;
        finished_processes = (pcb_t**)realloc(finished_processes, finished_capacity * sizeof(pcb_t*));
    }

    finished_processes[finished_count] = pcb;
    finished_count++;

    pthread_mutex_unlock(&metrics_mutex);
}

// Procesa e imprime el reporte de rendimiento digno de un sistema operativo real
void print_final_metrics() {
    pthread_mutex_lock(&metrics_mutex);

    if (finished_count == 0) {
        printf("\n[MÉTRICAS] No se ejecutó ningún proceso.\n");
        pthread_mutex_unlock(&metrics_mutex);
        return;
    }

    float total_waiting_time = 0;
    float total_turnaround_time = 0;
    float total_response_time = 0;

    printf("\n========================================================================\n");
    printf("               REPORTE FINAL DE RENDIMIENTO DEL MINIKERNEL              \n");
    printf("========================================================================\n");
    printf("PID\tBurst\tArrival\tCompletion\tTurnaround\tWaiting\tResponse\n");
    printf("------------------------------------------------------------------------\n");

    for (int i = 0; i < finished_count; i++) {
        pcb_t *p = finished_processes[i];
        printf("%d\t%dms\t%dms\t%dms\t\t%dms\t\t%dms\t%dms\n",
               p->pid, p->burst_time, p->arrival_time, p->completion_time,
               p->turnaround_time, p->waiting_time, p->response_time);

        total_waiting_time += p->waiting_time;
        total_turnaround_time += p->turnaround_time;
        total_response_time += p->response_time;
    }

    int total_time = get_current_simulation_time();
    float throughput = (float)finished_count / ((float)total_time / 100.0); // Escalado de tiempo

    printf("------------------------------------------------------------------------\n");
    printf("MÉTRICAS GLOBALES DEL SCHEDULER (Round Robin - Quantum: %dms):\n", QUANTUM);
    printf(" > Tiempo total de simulación:     %dms\n", total_time);
    printf(" > Procesos completados:          %d\n", finished_count);
    printf(" > Tiempo de Respuesta promedio:  %.2fms\n", total_response_time / finished_count);
    printf(" > Tiempo de Espera promedio:     %.2fms\n", total_waiting_time / finished_count);
    printf(" > Tiempo de Turnaround promedio: %.2fms\n", total_turnaround_time / finished_count);
    printf(" > Throughput del sistema:        %.3f procesos/unidad tiempo\n", throughput);
    printf("========================================================================\n");

    pthread_mutex_unlock(&metrics_mutex);
}

// Libera toda la memoria de los procesos recolectados
void metrics_free() {
    if (finished_processes) {
        for (int i = 0; i < finished_count; i++) {
            free(finished_processes[i]);
        }
        free(finished_processes);
    }
}
