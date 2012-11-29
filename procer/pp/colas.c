#include "commons/collections/sync_queue.h"
#include "commons/collections/dictionary.h"
#include "colas.h"
#include <stdio.h>
#include <stdlib.h>

t_sync_queue *cola_pendientes_nuevos;
t_sync_queue *cola_nuevos;
t_sync_queue *cola_pendientes_reanudar;
t_sync_queue *cola_reanudar;
t_sync_queue *cola_listos;
t_sync_queue *cola_fin_quantum;
t_sync_queue *cola_bloqueados;
t_sync_queue *cola_fin_bloqueados;
t_dictionary *tabla_suspendidos;
t_sync_queue *cola_fin_programa;
t_dictionary *tabla_procesos;
sem_t * mmp;
sem_t * semaforo_iot;

t_list * lista_auxiliar_prioridades;
bool (* algoritmo_ordenamiento)(void *, void *);
sem_t *mps;

void colas_initialize() {

	mmp = malloc(sizeof(sem_t));
	sem_init(mmp,0,3);

	semaforo_iot = malloc(sizeof(sem_t));
	sem_init(semaforo_iot,0,1);

	cola_pendientes_nuevos = sync_queue_create();
	cola_nuevos = sync_queue_create();
	cola_pendientes_reanudar = sync_queue_create();
	cola_reanudar = sync_queue_create();
	cola_listos = sync_queue_create();
	cola_fin_quantum = sync_queue_create();
	cola_bloqueados = sync_queue_create();
	cola_fin_bloqueados = sync_queue_create();
	tabla_suspendidos = dictionary_create(NULL);
	cola_fin_programa = sync_queue_create();

	tabla_procesos = dictionary_create(NULL);
	mps = malloc(sizeof(sem_t));
	// FIXME: parametrizar el 3
	sem_init(mps, 0, 3);

	cargar_lista_auxiliar_prioridades();
}


void *sacasaca(void *nada) {
	while(1) {
		sync_queue_pop(cola_pendientes_nuevos);
		printf("Saque uno de nuevos!\n");
	}
	return NULL;
}

void cargar_lista_auxiliar_prioridades(void){
	lista_auxiliar_prioridades = list_create();
	list_add(lista_auxiliar_prioridades,cola_nuevos);
	list_add(lista_auxiliar_prioridades,cola_reanudar);
	list_add(lista_auxiliar_prioridades,cola_fin_bloqueados);
	list_add(lista_auxiliar_prioridades,cola_fin_quantum);
}
