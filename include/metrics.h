#ifndef METRICS_H
#define METRICS_H

#include "pcb.h"
#include "queue.h"

// Puntero global para que todos los archivos puedan acceder a la cola de listos
extern queue_t *ready_queue;

// Funciones para el manejo del tiempo global simulado
void init_simulation_time();
void increment_simulation_time(int amount);
int get_current_simulation_time();

// Funciones para registrar y calcular el rendimiento
void metrics_init();
void register_finished_process(pcb_t *pcb);
void print_final_metrics();
void metrics_free();

#endif // METRICS_H
