#include "commons/collections/sync_queue.h"
#include "commons/collections/dictionary.h"
#include "colas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_sync_queue *cola_pendientes_nuevos;
t_sync_queue *cola_nuevos;
t_sync_queue *cola_pendientes_reanudar;
t_sync_queue *cola_reanudar;
t_sync_queue *cola_listos;
t_sync_queue *cola_fin_quantum;
t_sync_queue *cola_bloqueados;
t_sync_queue *cola_io;
t_sync_queue *cola_fin_bloqueados;
t_dictionary *tabla_suspendidos;
t_sync_queue *cola_fin_programa;
t_dictionary *tabla_procesos;
t_dictionary *diccionario_colas;
sem_t * mmp;

t_list * lista_auxiliar_prioridades;
bool (* algoritmo_ordenamiento)(void *, void *);
sem_t *mps;
int quantum = 0;

void colas_initialize() {

	cola_pendientes_nuevos = sync_queue_create();
	cola_nuevos = sync_queue_create();
	cola_pendientes_reanudar = sync_queue_create();
	cola_reanudar = sync_queue_create();
	cola_listos = sync_queue_create();
	cola_fin_quantum = sync_queue_create();
	cola_bloqueados = sync_queue_create();
	cola_io = sync_queue_create();
	cola_fin_bloqueados = sync_queue_create();
	tabla_suspendidos = dictionary_create(NULL);
	cola_fin_programa = sync_queue_create();

	inicializar_diccionario_colas();

	tabla_procesos = dictionary_create(NULL);

	cargar_lista_auxiliar_prioridades();
}


void cargar_lista_auxiliar_prioridades(void){
	lista_auxiliar_prioridades = list_create();
	list_add(lista_auxiliar_prioridades,cola_nuevos);
	list_add(lista_auxiliar_prioridades,cola_reanudar);
	list_add(lista_auxiliar_prioridades,cola_fin_bloqueados);
	list_add(lista_auxiliar_prioridades,cola_fin_quantum);
}

void inicializar_diccionario_colas(void){
	diccionario_colas = dictionary_create(NULL);
	dictionary_put(diccionario_colas,strdup("LPN"),cola_nuevos);
	dictionary_put(diccionario_colas,strdup("LPR"),cola_reanudar);
	dictionary_put(diccionario_colas,strdup("LFinIO"),cola_fin_bloqueados);
	dictionary_put(diccionario_colas,strdup("LFinQ"),cola_fin_quantum);
}
