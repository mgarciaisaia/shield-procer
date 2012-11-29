#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/network.h"
#include <errno.h>
#include <unistd.h>
#include "parser.h"
#include "server.h"
#include "colas.h"
#include <pthread.h>
#include "commons/collections/list.h"
#include "commons/string.h"
#include <sys/time.h>

void *pendientes_nuevos(void *nada) {
	int valor;
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_pendientes_nuevos);
		sem_wait(mmp);
		sync_queue_push(cola_nuevos,pcb);
		sem_getvalue(mmp,&valor);
		//todo: loguear
		printf("agregue un pcb: %d\n", valor);
	}
	return NULL;
}

void *finalizados(void *nada) {
	int contador;
	while(1) {
		t_pcb *pcb = sync_queue_pop(cola_fin_programa);
		
		char *pid;
		asprintf(&pid, "%d", pcb->id_proceso);
		dictionary_remove(tabla_procesos, pid);
		free(pid);
		
		sem_post(mps);
		sem_getvalue(mps, &contador);
		printf("elimine un pcb finalizado: %d\n", contador);
		
		char *mensaje_fin = strdup("0");
		concatenar_estado_pcb(&mensaje_fin, pcb);
		string_concat(&mensaje_fin, "\n\nProceso finalizado\n");
		socket_send(pcb->id_proceso, mensaje_fin, strlen(mensaje_fin) + 1);
		socket_send(pcb->id_proceso, "2", strlen("2") + 1);
		shutdown(pcb->id_proceso, SHUT_WR);
		
		destruir_pcb(pcb);
	}
	return NULL;
}

void * sts(void *);
void encolar_en_listos(void);
void encolar_lap_en_ll(void *);
void * procer(void *);
void * lanzar_ios(void *);
uint32_t no_encontro_pcb;


#define ERROR_BINDED 1
#define ERROR_LISTEN 2
#define ERROR_SEND_PID 3
#define ERROR_RECEIVE_PRIORITY 4
int main(void) {
    printf("Iniciado PROCER con PID %d\n", getpid());
	colas_initialize();
	
    #define THREAD_COUNT 6
	void *funciones_existentes[THREAD_COUNT] = { lts, pendientes_nuevos, sts,
		procer, finalizados, lanzar_ios };
	t_list *threads = list_create();
	pthread_t *thread;
	int index;
	for(index = 0; index < THREAD_COUNT; index++) {
		thread = malloc(sizeof(pthread_t));
	
		pthread_create(thread, NULL, funciones_existentes[index], NULL);
		list_add(threads, thread);
	}
	
	void *nada;
	
	void joinear_thread(void *thread) {
		pthread_join(*(pthread_t *)thread, &nada);
		free(thread);
	}
	
	list_iterate(threads, joinear_thread);
	return 0;
    int socket = socket_binded(23456);
    if(socket < 0) {
        printf("Error %d bindeando el socket: %s\n", errno, strerror(errno));
        exit(ERROR_LISTEN);
    }
    if(listen(socket, 0)) {
        printf("Error %d escuchando en el socket %d: %s\n", errno, socket, strerror(errno));
        exit(ERROR_LISTEN);
    }
    
    struct sockaddr_in address;
    socklen_t addressLength = sizeof address;
    
    int querySocket = accept(socket, (struct sockaddr *) &address, &addressLength);
    
    int sentBytes = socket_send(querySocket, &querySocket, sizeof(querySocket));
    if(sentBytes < 0) {
        perror("Error sending process' PID");
        exit(ERROR_SEND_PID);
    }
    
    void *input = NULL;
    int inputLenght = socket_receive(querySocket, &input);
    if(inputLenght != sizeof(uint8_t)) {
	    printf("Error recibiendo la prioridad del proceso %d - recibidos %d bytes, se esperaban %zd\n", querySocket, inputLenght, sizeof(uint8_t));
	    free(input);
	    exit(ERROR_RECEIVE_PRIORITY);
    }

    uint8_t prioridadProceso = *(uint8_t *) input;
    free(input);
    printf("La prioridad del proceso %d es %d\n", querySocket, prioridadProceso);

    inputLenght = socket_receive(querySocket, &input);
    printf("Recibido: %.*s\n", inputLenght, (char *)input);
    
    ejecutar((char *)input, querySocket, prioridadProceso);
    
    close(querySocket);
    close(socket);
    
    return (EXIT_SUCCESS);
}

void * sts(void * nada) {
	while(1){
		encolar_en_listos();
	}
	return NULL;
}

void encolar_en_listos(){
	no_encontro_pcb = 1; //PARA PODER SACAR SOLO UN ELEMENTO DE LA LISTAS DE PRIORIDADES
	list_iterate(lista_auxiliar_prioridades,encolar_lap_en_ll);
}

void encolar_lap_en_ll(void * reg_lista_void){
	t_sync_queue * sync_queue = reg_lista_void;
	if(!sync_queue_is_empty(sync_queue) && no_encontro_pcb){
		no_encontro_pcb = 0;
		t_pcb * pcb = sync_queue_pop(sync_queue);

		struct timeval tv;
		gettimeofday(&tv, NULL);
		double time_in_usec = (tv.tv_sec) * 1000000 + (tv.tv_usec);
		t_reg_listos * registro_listos = malloc(sizeof(t_reg_listos));
		registro_listos->pcb = pcb;
		registro_listos->tiempo_entrada_listos = time_in_usec;
		//todo: tener un ptr_a_funcion que me apunta al algoritmo de ordenamiento correspondiente
		//se usaría como 3er parámetro de sync_queue_ordered_insert

		sync_queue_ordered_insert(cola_listos,registro_listos,es_primer_pcb_mas_antiguo);
		printf("agarro uno de lista auxiliar de prioridades\n");
	}
}

void * procer(void * nada){
	while(1){
		t_reg_listos * registro_listo = sync_queue_pop(cola_listos);
		// todo: INICIALIZAR QUANTUM SI SE NECESITA
		int instrucciones_ejecutadas = 0;
		bool seguir_ejecutando = true;
		while(seguir_ejecutando){
			seguir_ejecutando = ejecutarInstruccion(registro_listo->pcb);
			//fixme: chequear suspendido
			//fixme: chequear quantum
			instrucciones_ejecutadas++;
		}
	}
	return NULL;
}

void * lanzar_ios(void * nada){
	while(1){
		void * registro_void_io = sync_queue_pop(cola_bloqueados);
	//	printf("%d\n",registro_io->tiempo_acceso_io);
		pthread_t * thr_ejecutar_io = malloc(sizeof(pthread_t));
		pthread_create(thr_ejecutar_io,NULL,ejecutar_io,registro_void_io);
	}
	return NULL;
}

void * ejecutar_io(void * void_pcb_io){
	t_registro_io * registro_io = (t_registro_io *) void_pcb_io;
	sleep(registro_io->tiempo_acceso_io);
	sync_queue_push(cola_fin_bloqueados,registro_io->pcb);
	sem_post(semaforo_iot);
	free(registro_io);
	return NULL;
}
