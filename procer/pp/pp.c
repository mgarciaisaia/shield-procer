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
#include "configuracion.h"
#include <sys/inotify.h>

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

void *pendientes_reanudar(void *nada) {
	int valor;
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_pendientes_reanudar);
		sem_wait(mmp);
		sync_queue_push(cola_reanudar,pcb);
		sem_getvalue(mmp,&valor);
		//todo: loguear
		printf("Pase el proceso %d de pendientes_reanudar a reanudar. MMP: %d\n", pcb->id_proceso, valor);
	}
	return NULL;
}

void *bloqueados_a_io(void *nada) {
	while(1) {
		void *registro_io = sync_queue_pop(cola_bloqueados);
		sem_wait(threads_iot);
		printf("El proceso %d va a IO desde bloqueados\n", ((t_registro_io *)registro_io)->pcb->id_proceso);
		sync_queue_push(cola_io, registro_io);
	}
}

void *finalizados(void *nada) {
	int contador;
	while(1) {
		t_pcb *pcb = sync_queue_pop(cola_fin_programa);
		
		char *pid = pid_string(pcb->id_proceso);
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
void * monitorear_configuracion(void *);
uint32_t no_encontro_pcb;


#define ERROR_BINDED 1
#define ERROR_LISTEN 2
#define ERROR_SEND_PID 3
#define ERROR_RECEIVE_PRIORITY 4
int main(void) {
    printf("Iniciado PROCER con PID %d\n", getpid());

	colas_initialize();

    inicializar_configuracion();

	registrarSignalListener();
	
    #define THREAD_COUNT 9
	void *funciones_existentes[THREAD_COUNT] = { lts, pendientes_nuevos, sts,
		procer, finalizados, lanzar_ios, pendientes_reanudar,
		monitorear_configuracion, bloqueados_a_io };
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
	return EXIT_SUCCESS;
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

		sync_queue_ordered_insert(cola_listos,registro_listos,algoritmo_ordenamiento);
		printf("agarro uno de lista auxiliar de prioridades\n");
	}
}

void suspender_proceso(t_pcb *pcb) {
	char *pid = pid_string(pcb->id_proceso);
	printf("Suspendo el proceso %s\n", pid);
	dictionary_put(tabla_suspendidos, pid, pcb);
	free(pid);

	sem_post(mmp);

	// mensaje con 1 hace que el PI pida al usuario reanudar
	char *mensaje = strdup("1");
	concatenar_estado_pcb(&mensaje, pcb);
	string_concat(&mensaje, "\n\nProceso suspendido\n");

	socket_send(pcb->id_proceso, mensaje, strlen(mensaje));
}

void * procer(void * nada){
	while(1){
		t_reg_listos * registro_listo = sync_queue_pop(cola_listos);
		t_pcb *pcb = registro_listo->pcb;
		int pid_proceso = pcb->id_proceso;
		free(registro_listo);
		int instrucciones_ejecutadas = 0;
		bool seguir_ejecutando = true;
		while(seguir_ejecutando){
			seguir_ejecutando = ejecutarInstruccion(pcb);
			if(seguir_ejecutando && hayQueSuspenderProceso) {
				suspender_proceso(pcb);
				
				hayQueSuspenderProceso = 0;
				seguir_ejecutando = false;
			} else if(seguir_ejecutando && quantum && instrucciones_ejecutadas >= quantum - 1) {
				sync_queue_push(cola_fin_quantum, pcb);
				seguir_ejecutando = false;
			}
			instrucciones_ejecutadas++;
		}
		printf("Ejecute %d instrucciones del proceso %d\n", instrucciones_ejecutadas, pid_proceso);
	}
	return NULL;
}

void * lanzar_ios(void * nada){
	while(1){
		void * registro_void_io = sync_queue_pop(cola_io);
		printf("Creo el hilo de IO de %d\n", ((t_registro_io *)registro_void_io)->pcb->id_proceso);
		pthread_t * thr_ejecutar_io = malloc(sizeof(pthread_t));
		pthread_create(thr_ejecutar_io,NULL,ejecutar_io,registro_void_io);
	}
	return NULL;
}

void * ejecutar_io(void * void_pcb_io){
	t_registro_io * registro_io = (t_registro_io *) void_pcb_io;
	printf("Ejecuto IO para %d por %d segundos\n", registro_io->pcb->id_proceso, registro_io->tiempo_acceso_io);
	sleep(registro_io->tiempo_acceso_io);
	printf("Termine IO para %d por %d segundos\n", registro_io->pcb->id_proceso, registro_io->tiempo_acceso_io);
	sem_post(threads_iot);
	sync_queue_push(cola_fin_bloqueados,registro_io->pcb);
	free(registro_io);
	return NULL;
}


void * monitorear_configuracion(void * nada){

	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor,PATH_CONFIG,IN_MODIFY);

	while (1) {

	// El file descriptor creado por inotify, es el que recibe la información sobre los eventos ocurridos
	// para leer esta información el descriptor se lee como si fuera un archivo comun y corriente pero
	// la diferencia esta en que lo que leemos no es el contenido de un archivo sino la información
	// referente a los eventos ocurridos
		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}

		int offset = 0;

		// Luego del read buffer es un array de n posiciones donde cada posición contiene
		// un eventos ( inotify_event ) junto con el nombre de este.
		while (offset < length) {

			// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
			// nombres pueden tener nombres mas cortos que 24 caracteres el tamaño va a ser menor
			// a sizeof( struct inotify_event ) + 24.
			struct inotify_event *event =
					(struct inotify_event *) &buffer[offset];

			if (event->mask & IN_MODIFY) {
				if (event->mask & IN_ISDIR) {
					printf("The directory %s was modified.\n", event->name);
				} else {
					printf("se produjo un cambio en el archivo de configuracion, volvio a cargar\n");
					config = config_create(PATH_CONFIG);
					asignar_parametros_que_cambian_en_tiempo_de_ejecucion(config);
					config_destroy(config);
					sync_queue_sort(cola_listos,algoritmo_ordenamiento);
					//FIXME: preguntar para que entre una sola vez aca al modificar el archivo de conf
				}
			}
			offset += sizeof(struct inotify_event) + event->len;
		}
	}
	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);

	return NULL;
}
