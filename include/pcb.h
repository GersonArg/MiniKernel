#ifndef PCB_H
#define PCB_H

// Definición de los estados del proceso para mayor claridad en los logs
typedef enum {
    STATE_READY,
    STATE_RUNNING,
    STATE_TERMINATED
} process_state_t;

// Estructura del Process Control Block (PCB) según el enunciado
typedef struct {
    int pid;            // Identificador único del proceso
    int burst_time;     // Tiempo total de CPU requerido originalmente
    int remaining_time; // Tiempo de CPU que le falta para terminar
    int priority;       // Prioridad del proceso
    int arrival_time;   // Tiempo de simulación en el que llegó a la cola
    int state;          // Estado actual (usando los enums de arriba)

    // Métricas individuales útiles para el cálculo final
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;  // Tiempo desde que llega hasta que pisa la CPU por primera vez
    int first_time_served; // Bandera para saber si ya pasó por la CPU alguna vez
} pcb_t;

#endif // PCB_H
