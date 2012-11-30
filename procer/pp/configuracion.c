/*
 * configuracion.c
 *
 *  Created on: 29/11/2012
 *      Author: utnso
 */

#include "commons/config.h"
#include "configuracion.h"
#include "colas.h"
#include <semaphore.h>
#include <limits.h>
#include <stdlib.h>
#include "commons/string.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include "commons/log.h"

t_config * config;
char *puerto_tcp;
int time_sleep;
int time_io;
sem_t * threads_iot;
t_log *logger;

void inicializar_configuracion(){
	t_config * config = config_create(PATH_CONFIG);
	
	char *path_archivo_configuracion = NULL;
	if((path_archivo_configuracion = realpath(PATH_CONFIG, path_archivo_configuracion)) == NULL) {
		perror("realpath del archivo de configuracion");
		printf("El path de configuracion es: %s\n", PATH_CONFIG);
		exit(1);
	}
	
	if(config == NULL) {
		printf("No se pudo leer el archivo de configuracion %s\n", path_archivo_configuracion);
		exit(2);
	}

	mps = malloc(sizeof(sem_t));
	sem_init(mps,0,config_get_int_value(config,"MPS"));

	mmp = malloc(sizeof(sem_t));
	sem_init(mmp,0,config_get_int_value(config,"MMP"));

	threads_iot=malloc(sizeof(sem_t));
	sem_init(threads_iot,0,config_get_int_value(config,"THREADS_IOT"));
	int cantidad_hilos_io;
	sem_getvalue(threads_iot, &cantidad_hilos_io);
	printf("Threads IO: %d\n", cantidad_hilos_io);

	puerto_tcp = strdup(config_get_string_value(config,"PUERTO_TCP"));
	printf("El puerto TCP es %s\n", puerto_tcp);
	time_io = config_get_int_value(config,"TIME_IO");
	asignar_parametros_que_cambian_en_tiempo_de_ejecucion(config);
	config_destroy(config);
	
	printf("Termino de leer el archivo de configuracion %s\n", path_archivo_configuracion);
	free(path_archivo_configuracion);
}

void asignar_parametros_que_cambian_en_tiempo_de_ejecucion(t_config * config){
	asignar_algoritmo_de_ordenamiento(config);
	asignar_lista_lap(config);
	time_sleep = config_get_int_value(config,"TIME_SLEEP");
}

void asignar_algoritmo_de_ordenamiento(t_config * config){
	char * string_algoritmo = config_get_string_value(config,"ALGORITMO");
	if(string_equals_ignore_case(string_algoritmo,"FIFO")
			|| string_equals_ignore_case(string_algoritmo,"RR")){
		algoritmo_ordenamiento = es_primer_pcb_mas_antiguo;
	} else if(string_equals_ignore_case(string_algoritmo,"PRI")){
		algoritmo_ordenamiento = es_primer_pcb_de_menor_prioridad;
	} else if(string_equals_ignore_case(string_algoritmo,"SPN")){
		algoritmo_ordenamiento = es_primer_pcb_de_rafaga_mas_corta;
	}
}

void asignar_lista_lap(t_config * config){

	int prioridad;
	t_sync_queue * ptr_cola_sincronizada;

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LPN");
	prioridad = config_get_int_value(config,"PRI_LPN");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LPR");
	prioridad = config_get_int_value(config,"PRI_LPR");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LFinQ");
	prioridad = config_get_int_value(config,"PRI_LFINQ");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LFinIO");
	prioridad = config_get_int_value(config,"PRI_LFINIO");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);

}
