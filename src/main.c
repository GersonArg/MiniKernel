#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "pcb.h"
#include "queue.h"
#include "sync.h"
#include "metrics.h"

// Declaración de la función que ejecutan los hilos CPU (definida en cpu.c)
extern void* cpu_worker(void *arg);

// --- HILO GENERADOR DE PROCESOS ---
void* process_generator(void *arg) {
    (void)arg; // Evita el warning de parámetro no usado de forma elegante
    printf("[GENERADOR] Hilo inicializado. Creando %d procesos simulados...\n", MAX_PROCESSES);

    // Inicializamos la semilla para los números aleatorios
    srand(time(NULL));

    for (int i = 1; i <= MAX_PROCESSES; i++) {
        // 1. Duerme un tiempo aleatorio antes de que llegue un nuevo proceso (entre 10ms y 50ms simulados)
        usleep((rand() % 5 + 1) * 10000);

        // 2. Asignamos memoria para el nuevo PCB
        pcb_t *new_process = (pcb_t*)malloc(sizeof(pcb_t));
        if (!new_process) {
            perror("Error al asignar memoria para un nuevo PCB");
            exit(EXIT_FAILURE);
        }

        // 3. Asignación de atributos de forma aleatoria según los requisitos
        new_process->pid = i;
        new_process->burst_time = rand() % 10 + 2; // Ráfaga de CPU aleatoria entre 2 y 11 unidades
        new_process->remaining_time = new_process->burst_time; // Al inicio es igual al total
        new_process->priority = rand() % 5 + 1;    // Prioridades del 1 al 5
        new_process->arrival_time = get_current_simulation_time(); // Momento exacto de llegada
        new_process->state = STATE_READY;
        new_process->first_time_served = 0; // Bandera de control inicializada en falso

        // 4. Inserta de forma segura el proceso en la cola de listos (Ready Queue)
        enqueue(ready_queue, new_process);
    }

    printf("[GENERADOR] Se han creado todos los procesos programados. Cerrando ciclo de entrada.\n");

    // --- APAGADO SEGURO DE LA SIMULACIÓN ---
    pthread_mutex_lock(&queue_mutex);
    simulation_active = 0; // Indicamos a las CPUs que el generador cerró ventanilla
    pthread_cond_broadcast(&queue_cond); // Despertamos a TODAS las CPUs que estén durmiendo en la cola vacía
    pthread_mutex_unlock(&queue_mutex);

    pthread_exit(NULL);
}

// --- FUNCIÓN PRINCIPAL (MAIN) ---
int main() {
    printf("Initializing MiniKernel OS Simulation...\n");
    printf("Configuración: %d CPUs | Round Robin Quantum: %dms\n\n", N_CPUS, QUANTUM);

    // Inicialización del sistema
    init_simulation_time();
    metrics_init();
    ready_queue = queue_init();

    // Declaración de los identificadores de hilos
    pthread_t generator_thread;
    pthread_t cpu_threads[N_CPUS];

    // 1. Crear el hilo Generador de Procesos
    if (pthread_create(&generator_thread, NULL, process_generator, NULL) != 0) {
        perror("Error al crear el hilo generador de procesos");
        return EXIT_FAILURE;
    }

    // 2. Crear los hilos que simularán las CPUs independientes
    for (int i = 0; i < N_CPUS; i++) {
        int *cpu_id = (int*)malloc(sizeof(int)); // Memoria dinámica exclusiva para pasar el ID sin colisiones
        *cpu_id = i + 1;
        if (pthread_create(&cpu_threads[i], NULL, cpu_worker, (void*)cpu_id) != 0) {
            perror("Error al crear hilo de CPU");
            return EXIT_FAILURE;
        }
    }

    // --- ESPERA EN CADENA (JOIN) ---
    // Esperamos a que el generador termine de meter los procesos
    pthread_join(generator_thread, NULL);

    // Esperamos a que cada CPU procese todo lo que quede en la cola y se apague
    for (int i = 0; i < N_CPUS; i++) {
        pthread_join(cpu_threads[i], NULL);
    }

    // 3. Imprimir las métricas finales recopiladas (Exigido en el PDF)
    print_final_metrics();

    // Limpieza absoluta de la memoria para evitar memory leaks
    metrics_free();
    queue_free(ready_queue);

    printf("\nMiniKernel se ha apagado limpiamente. Proyecto ejecutado con éxito.\n");
    return EXIT_SUCCESS;
}
