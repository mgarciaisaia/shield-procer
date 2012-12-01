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
#include "commons/log.h"

void *pendientes_nuevos(void *nada) {
	log_debug(logger, "Inicia el thread de pendientes_nuevos a nuevos");
	int valor;
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_pendientes_nuevos);
		log_debug(logger, "Saco el proceso %d de pendientes_nuevos", pcb->id_proceso);
		sem_wait(mmp);
		sync_queue_push(cola_nuevos,pcb);
		sem_getvalue(mmp,&valor);
		log_lsch(logger, "Muevo el proceso %d de pendientes_nuevos a nuevos. MMP: %d", pcb->id_proceso, valor);
	}
	return NULL;
}

void *pendientes_reanudar(void *nada) {
	log_debug(logger, "Inicia el thread de pendientes_reanudar a reanudar");
	int valor;
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_pendientes_reanudar);
		log_debug(logger, "Saco el proceso %d de pendientes_reanudar", pcb->id_proceso);
		sem_wait(mmp);
		sync_queue_push(cola_reanudar,pcb);
		sem_getvalue(mmp,&valor);
		log_lsch(logger, "Muevo el proceso %d de pendientes_reanudar a reanudar. MMP: %d", pcb->id_proceso, valor);
	}
	return NULL;
}

void *bloqueados_a_io(void *nada) {
	log_debug(logger, "Inicia el thread de bloqueados a entrada/salida");
	while(1) {
		t_registro_io *registro_io = sync_queue_pop(cola_bloqueados);
		log_debug(logger, "Saco el proceso %d de bloqueados", registro_io->pcb->id_proceso);
		sem_wait(threads_iot);
		sync_queue_push(cola_io, registro_io);
		log_lsch(logger, "Muevo el proceso %d de bloqueados a IO", registro_io->pcb->id_proceso);
	}
}

void *finalizados(void *nada) {
	log_debug(logger, "Inicia el thread consumidor de finalizados");
	int contador;
	while(1) {
		t_pcb *pcb = sync_queue_pop(cola_fin_programa);
		log_debug(logger, "Saco el proceso %d de finalizados", pcb->id_proceso);
		
		char *pid = pid_string(pcb->id_proceso);
		dictionary_remove(tabla_procesos, pid);
		free(pid);
		
		sem_post(mps);
		sem_getvalue(mps, &contador);
		log_lsch(logger, "Saco el proceso %d de finalizados y lo elimino del sistema", pcb->id_proceso);
		
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
	logger = log_create("PP.log", "PP", true, LOG_LEVEL_TRACE);
    log_info(logger, "Iniciado PROCER con PID %d", getpid());

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
	log_debug(logger, "Inicio el thread del STS");
	while(1){
		encolar_en_listos();
	}
	return NULL;
}

void encolar_en_listos(){
	no_encontro_pcb = 1; //PARA PODER SACAR SOLO UN ELEMENTO DE LA LISTAS DE PRIORIDADES

	void encolar_lap_en_ll(void * reg_lista_void) {
		t_sync_queue * sync_queue = (t_sync_queue *) reg_lista_void;
		if(no_encontro_pcb) {
			t_pcb *pcb = sync_queue_try_pop(sync_queue);
			if(pcb != NULL) {
				no_encontro_pcb = 0;
				struct timeval tv;
				gettimeofday(&tv, NULL);
				double time_in_usec = (tv.tv_sec) * 1000000 + (tv.tv_usec);
				t_reg_listos * registro_listos = malloc(sizeof(t_reg_listos));
				registro_listos->pcb = pcb;
				registro_listos->tiempo_entrada_listos = time_in_usec;
				log_lsch(logger, "STS: saco el proceso %d y lo mando a ready", pcb->id_proceso);

				sync_queue_ordered_insert(cola_listos,registro_listos,algoritmo_ordenamiento);
			}
		}
	}

	list_iterate(lista_auxiliar_prioridades,encolar_lap_en_ll);
}

void suspender_proceso(t_pcb *pcb) {
	char *pid = pid_string(pcb->id_proceso);
	dictionary_put(tabla_suspendidos, pid, pcb);

	int valor_mmp;
	sem_post(mmp);
	sem_getvalue(mmp, &valor_mmp);
	log_lsch(logger, "Suspendo el proceso %s. MMP: %d", pid, valor_mmp);

	// mensaje con 1 hace que el PI pida al usuario reanudar
	char *mensaje = strdup("1");
	concatenar_estado_pcb(&mensaje, pcb);
	string_concat(&mensaje, "\n\nProceso suspendido\n");

	socket_send(pcb->id_proceso, mensaje, strlen(mensaje));
}

void * procer(void * nada){
	log_debug(logger, "Inicio el hilo procer");
	while(1){
		t_reg_listos * registro_listo = sync_queue_pop(cola_listos);
		t_pcb *pcb = registro_listo->pcb;
		int pid_proceso = pcb->id_proceso;
		log_lsch(logger, "Saco el proceso %d de listos y lo ejecuto", pid_proceso);
		free(registro_listo);
		pcb->valor_estimacion_anterior = calcular_rafaga(pcb->valor_estimacion_anterior, pcb->ultima_rafaga);
		pcb->ultima_rafaga = 0;
		bool seguir_ejecutando = true;
		hayQueSuspenderProceso = 0;
		while(seguir_ejecutando){
			seguir_ejecutando = ejecutarInstruccion(pcb);
			if(seguir_ejecutando && hayQueSuspenderProceso) {
				suspender_proceso(pcb);
				
				hayQueSuspenderProceso = 0;
				seguir_ejecutando = false;
			} else if(seguir_ejecutando && quantum && pcb->ultima_rafaga >= quantum - 1) {
				sync_queue_push(cola_fin_quantum, pcb);
				log_lsch(logger, "Paso el proceso %d de ejecucion a fin de quantum", pid_proceso);
				seguir_ejecutando = false;
			}
			pcb->ultima_rafaga++;
		}
		log_debug(logger, "Ejecute %d instrucciones del proceso %d", pcb->ultima_rafaga, pid_proceso);
	}
	return NULL;
}

void * lanzar_ios(void * nada){
	log_debug(logger, "Inicio el hilo lanzador de E/S");
	while(1){
		void * registro_void_io = sync_queue_pop(cola_io);
		log_lsch(logger, "Saco el proceso %d de la cola E/S y ejecuto la E/S",
				((t_registro_io *)registro_void_io)->pcb->id_proceso);
		pthread_t * thr_ejecutar_io = malloc(sizeof(pthread_t));
		pthread_create(thr_ejecutar_io,NULL,ejecutar_io,registro_void_io);
		free(thr_ejecutar_io);
	}
	return NULL;
}

void * ejecutar_io(void * void_pcb_io){
	t_registro_io * registro_io = (t_registro_io *) void_pcb_io;
	int tiempo_sleep = registro_io->tiempo_acceso_io * time_io;
	log_debug(logger, "Ejecuto %d unidades de IO para %d (%d segundos)",
			registro_io->tiempo_acceso_io, registro_io->pcb->id_proceso, tiempo_sleep);
	sleep(tiempo_sleep);
	log_debug(logger, "Termine %d unidades de IO para %d (%d segundos)",
			registro_io->tiempo_acceso_io, registro_io->pcb->id_proceso, tiempo_sleep);
	sem_post(threads_iot);
	sem_getvalue(threads_iot, &tiempo_sleep);
	log_lsch(logger,
		"Termino una IO y mando el proceso %d a fin bloqueados. Threads IO libres: %d",
			registro_io->pcb->id_proceso, tiempo_sleep);
	sync_queue_push(cola_fin_bloqueados,registro_io->pcb);
	free(registro_io);
	return NULL;
}


void * monitorear_configuracion(void * nada){

	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		log_error(logger, "Error %d inicializando inotify: %s", errno, strerror(errno));
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
			log_error(logger, "Error %d leyendo el file descriptor de inotify: %s", errno, strerror(errno));
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
				if (!(event->mask & IN_ISDIR)) {
					log_info(logger, "Cambios detectados en el archivo de configuracion. Recargando...");
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
