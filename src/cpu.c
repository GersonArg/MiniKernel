#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.h"
#include "sync.h"
#include "metrics.h"

// Función que ejecutará cada hilo de la CPU
void* cpu_worker(void *arg) {
    int cpu_id = *(int*)arg;
    free(arg); // Liberamos la memoria del ID dinámico asignado en main

    printf("[CPU %d] Iniciada y lista para planificar procesos.\n", cpu_id);

    // Bucle principal de la CPU: se ejecuta mientras haya procesos o la simulación siga activa
    while (1) {
        // Extrae de forma segura un proceso (bloqueante)
        pcb_t *process = dequeue(ready_queue);

        // Si dequeue retorna NULL, significa que la simulación terminó y la cola está vacía
        if (process == NULL) {
            break;
        }

        // --- INICIO CAMBIO DE CONTEXTO / EJECUCIÓN ---
        process->state = STATE_RUNNING;

        // Registrar tiempo de respuesta (Response Time) si es la primera vez que usa la CPU
        if (!process->first_time_served) {
            process->response_time = get_current_simulation_time() - process->arrival_time;
            process->first_time_served = 1;
        }

        // Determinar cuánto tiempo se va a ejecutar en este turno (Mínimo entre Quantum y Tiempo Restante)
        int time_to_execute = (process->remaining_time > QUANTUM) ? QUANTUM : process->remaining_time;

        printf("[CPU %d] Ejecutando PID %d por %dms (Restante original: %dms)\n",
               cpu_id, process->pid, time_to_execute, process->remaining_time);

        // Simula el procesamiento en la CPU durmiendo el hilo (10ms reales por cada unidad de simulación para velocidad)
        usleep(time_to_execute * 10000);

        // Actualizamos el reloj global de la simulación y el tiempo restante del proceso
        increment_simulation_time(time_to_execute);
        process->remaining_time -= time_to_execute;

        // --- FIN DE LA EJECUCIÓN DEL TURNO ---
        if (process->remaining_time > 0) {
            // El proceso no terminó en su Quantum: Cambio de contexto hacia la cola de listos
            process->state = STATE_READY;
            printf("[CPU %d] Quantum expirado para PID %d. Reinsertando en cola.\n", cpu_id, process->pid);
            enqueue(ready_queue, process);
        } else {
            // El proceso ha terminado su ejecución con éxito
            process->state = STATE_TERMINATED;
            process->completion_time = get_current_simulation_time();
            process->turnaround_time = process->completion_time - process->arrival_time;
            process->waiting_time = process->turnaround_time - process->burst_time;

            printf("[CPU %d] ¡Proceso PID %d TERMINADO! (Turnaround: %dms, Espera: %dms)\n",
                   cpu_id, process->pid, process->turnaround_time, process->waiting_time);

            // Enviar el proceso terminado al módulo de métricas antes de que se pierda
            register_finished_process(process);
        }
    }

    printf("[CPU %d] Apagándose ordenadamente.\n", cpu_id);
    pthread_exit(NULL);
}
