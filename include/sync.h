#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

// Mutex global para proteger la cola de listos y variables compartidas
extern pthread_mutex_t queue_mutex;

// Variable condicional para avisar a las CPUs cuando hay procesos disponibles
extern pthread_cond_t queue_cond;

// Bandera global para detener de forma segura la simulación (cuando el generador termine)
extern int simulation_active;

// Constantes del sistema para facilitar pruebas
#define N_CPUS 2          // Número de hilos CPU simulados
#define QUANTUM 3         // Tiempo máximo continuo en CPU (Round Robin)
#define MAX_PROCESSES 15  // Total de procesos que creará el generador en la prueba

#endif // SYNC_H
