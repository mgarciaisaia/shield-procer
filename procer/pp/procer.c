#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/list.h"
#include "parser.h"

t_pcb *sacar_proceso_listo();
int agregar_proceso_listo(t_pcb *);
void *producirSiempre();
void *consumirSiempre();

sem_t semaforo_procesos_listos;
pthread_rwlock_t lock_procesos_listos;
t_list *lista_listos;
int quantum_sistema; // quantum para los procesos del sistema, viene del archivo de configuracion

int main(void) {
    lista_listos = list_create();
    pthread_rwlock_init(&lock_procesos_listos, NULL);
    sem_init(&semaforo_procesos_listos, 0, 0);
    pthread_t productor, consumidor;
    pthread_create(&productor, NULL, producirSiempre, NULL);
    pthread_create(&consumidor, NULL, consumirSiempre, NULL);
    void *nada = malloc(sizeof(int));
    pthread_join(productor, &nada);
    pthread_join(consumidor, &nada);
    return EXIT_SUCCESS;
}

void *producirSiempre() {
    while(1) {
        t_pcb *pcb = malloc(sizeof(t_pcb));
        agregar_proceso_listo(pcb);
        printf("Agregué un proceso listo\n");
    }
    return 0;
}

void *consumirSiempre() {
    while(1) {
        t_pcb *pcb = sacar_proceso_listo();
        free(pcb);
        printf("Saqué un proceso listo\n");
    }
    return 0;
}

int procer() {
    while(1) {
        t_pcb *proceso = sacar_proceso_listo();
        int quantum = 0, seguir_ejecutando = 1;
        while(quantum < quantum_sistema && seguir_ejecutando) {
            ejecutarInstruccion(proceso);
            quantum++;
        }
    }
}

t_pcb *sacar_proceso_listo() {
    sem_wait(&semaforo_procesos_listos);
    pthread_rwlock_wrlock(&lock_procesos_listos);
    t_pcb *pcb = list_remove(lista_listos, 0);
    pthread_rwlock_unlock(&lock_procesos_listos);
    return pcb;
}

int agregar_proceso_listo(t_pcb *pcb) {
    pthread_rwlock_wrlock(&lock_procesos_listos);
    list_add(lista_listos, pcb);
    sem_post(&semaforo_procesos_listos);
    pthread_rwlock_unlock(&lock_procesos_listos);
    return 0;
}
