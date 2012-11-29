/*
 * sincronizacion.c
 *
 *  Created on: 26/11/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "prueba_listas.h"

#include "commons/collections/sync_queue.h"
#include "colas.h"

t_list * lista_procesos_nuevos;
t_list * lista_procesos_reanudados;
t_list * lista_fin_quantum;
t_list * lista_fin_io;
t_list * lista_de_listos;

//t_list * lista_auxiliar_prioridades;

t_dictionary * diccionario_listas;

pthread_mutex_t mutex_LPN = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_LPR = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_LFinQ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_LFinIO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_de_listos = PTHREAD_MUTEX_INITIALIZER;

sem_t contador_lista_listos;

uint32_t no_encontro_pcb;

int main(void) {

	inicializar_listas();
	inicializar_diccionario();

	pthread_t thr_LTS;
	pthread_t thr_STS;
	pthread_t thr_PROCER;
	sem_init(&contador_lista_listos, 0, 0); // <-- SE REFIERE AL NIVEL DE MULTIPROGRAMACION

	pthread_create(&thr_LTS, NULL, lts, NULL);
	pthread_create(&thr_STS, NULL, sts, NULL);
	pthread_create(&thr_PROCER, NULL, procer, NULL);
	void *nada = malloc(sizeof(int));
	pthread_join(thr_LTS, &nada);
	pthread_join(thr_STS, &nada);
	pthread_join(thr_PROCER, &nada);

	destruir_listas();
	destruir_diccionario();

	return EXIT_SUCCESS;
}

void inicializar_listas(){
	lista_procesos_nuevos = list_create();
	lista_procesos_reanudados = list_create();
	lista_fin_quantum = list_create();
	lista_fin_io = list_create();
	lista_de_listos = list_create();

	inicializar_lista_auxiliar_prioridades();
}

void destruir_listas(){
	list_destroy(lista_procesos_nuevos);
	list_destroy(lista_procesos_reanudados);
	list_destroy(lista_fin_quantum);
	list_destroy(lista_fin_io);
}

void inicializar_diccionario(){
	diccionario_listas = dictionary_create(NULL);
	cargar_listas_en_diccionario();
}

void destruir_diccionario(){
	dictionary_destroy(diccionario_listas);
}

/*
 * ME SIRVE PARA PODER UBICAR LA LISTA + SEMAFORO, CUANDO LO NECESITE PARA ORDENAR LAP(lista auxiliar de prioridades)
 */
void cargar_listas_en_diccionario(){
	t_reg_lista * reg_lista = malloc(sizeof(t_reg_lista));
	reg_lista->lista = lista_procesos_nuevos;
	reg_lista->sem_mutex = &mutex_LPN;
	dictionary_put(diccionario_listas,strdup("LPN"),reg_lista);

	t_reg_lista * reg_lista_2 = malloc(sizeof(t_reg_lista));
	reg_lista_2->lista = lista_procesos_reanudados;
	reg_lista_2->sem_mutex = &mutex_LPR;
	dictionary_put(diccionario_listas,strdup("LPR"),reg_lista_2);

	t_reg_lista * reg_lista_3 = malloc(sizeof(t_reg_lista));
	reg_lista_3->lista = lista_fin_io;
	reg_lista_3->sem_mutex = &mutex_LFinIO;
	dictionary_put(diccionario_listas,strdup("LFinIO"),reg_lista_3);

	t_reg_lista * reg_lista_4 = malloc(sizeof(t_reg_lista));
	reg_lista_4->lista = lista_fin_quantum;
	reg_lista_4->sem_mutex = &mutex_LFinQ;
	dictionary_put(diccionario_listas,strdup("LFinQ"),reg_lista_4);
}

void inicializar_lista_auxiliar_prioridades(){

/*
 * Reservo los espacios de la lista auxiliar, para después poder hacer un list_replace usando como índice la prioridad
 */

	lista_auxiliar_prioridades = list_create();
	list_add(lista_auxiliar_prioridades,NULL);
	list_add(lista_auxiliar_prioridades,NULL);
	list_add(lista_auxiliar_prioridades,NULL);
	list_add(lista_auxiliar_prioridades,NULL);
}

/*
 * DE ACUERDO A LA PRIORIDAD DE LA COLA, SE LA CARGA EN LAP
 */

void cargar_listas_a_LAP(){
	// todo: tengo q sacar el tipo de lista y prioridad del archivo de configuracion
	char * tipo_lista = "LFinQ";
	t_reg_lista * reg_lista = dictionary_get(diccionario_listas,tipo_lista);
	list_replace(lista_auxiliar_prioridades,0,reg_lista);
	tipo_lista = "LFinIO";
	reg_lista = dictionary_get(diccionario_listas,tipo_lista);
	list_replace(lista_auxiliar_prioridades,1,reg_lista);
	tipo_lista = "LPN";
	reg_lista = dictionary_get(diccionario_listas,tipo_lista);
	list_replace(lista_auxiliar_prioridades,2,reg_lista);
	tipo_lista = "LPR";
	reg_lista = dictionary_get(diccionario_listas,tipo_lista);
	list_replace(lista_auxiliar_prioridades,3,reg_lista);
}

int sts() {
	while(1){
		encolar_en_listos();
	}
	return EXIT_SUCCESS;
}

void encolar_en_listos(){
	no_encontro_pcb = 1; //PARA PODER SACAR SOLO UN ELEMENTO DE LA LISTAS DE PRIORIDADES
	// todo: cargar la lista_auxiliar_prioridades
	list_iterate(lista_auxiliar_prioridades,encolar_lap_en_ll);
}

void encolar_lap_en_ll(void * reg_lista_void){
	t_sync_queue * sync_queue = reg_lista_void;
	while(!list_is_empty(sync_queue->queue) && no_encontro_pcb){
		no_encontro_pcb = 0;
		t_pcb * pcb = sync_queue_pop(sync_queue);
		sync_queue_ordered_insert(cola_listos,pcb,algoritmo_ordenamiento);
	}
}

t_pcb * sacar_proceso(t_list * lista, pthread_mutex_t * sem_mutex) {
	pthread_mutex_lock(sem_mutex);
	t_pcb * pcb = list_remove(lista,0);
	pthread_mutex_unlock(sem_mutex);
	return pcb;
}

void agregar_proceso(t_list * lista, pthread_mutex_t * sem_mutex, t_pcb *pcb) {
	pthread_mutex_lock(sem_mutex);
	list_add(lista, pcb);
	pthread_mutex_unlock(sem_mutex);
}

void * procer(void * nada){
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_listos);
		// todo: INICIALIZAR QUANTUM SI SE NECESITA
//		procesar(pcb);
	}
	return NULL;
}
