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
#include <errno.h>

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
		log_error(logger, "Error %d obteniendo el realpath del archivo de configuracion [%s]: %s",
				errno, PATH_CONFIG, strerror(errno));
		exit(1);
	}
	
	if(config == NULL) {
		log_error(logger, "No se pudo leer el archivo de configuracion %s", path_archivo_configuracion);
		exit(2);
	}

	int valor;
	
	valor = config_get_int_value(config, "ACTIVAR_CONSOLA_LOG");
	log_set_is_active_console(logger, valor);
	
	char *nivel_log = config_get_string_value(config, "NIVEL_LOG");
	log_set_detail_level(logger, log_level_from_string(nivel_log));
	
	log_info(logger, "Consola de log activada: %s", valor ? "Sí" : "No");
	log_info(logger, "Nivel de log: %s", nivel_log);
	
	mps = malloc(sizeof(sem_t));
	valor = config_get_int_value(config,"MPS");
	sem_init(mps,0,valor);
	log_debug(logger, "Inicializado MPS con valor %d", valor);

	mmp = malloc(sizeof(sem_t));
	valor = config_get_int_value(config,"MMP");
	sem_init(mmp,0,valor);
	log_debug(logger, "Inicializado MMP con valor %d", valor);

	threads_iot=malloc(sizeof(sem_t));
	valor = config_get_int_value(config,"THREADS_IOT");
	sem_init(threads_iot,0,valor);
	log_debug(logger, "Inicializado Threads IO con valor %d", valor);
	
	puerto_tcp = strdup(config_get_string_value(config,"PUERTO_TCP"));
	log_debug(logger, "El puerto TCP es %s", puerto_tcp);
	
	time_io = config_get_int_value(config,"TIME_IO");
	log_debug(logger, "Inicializado el retardo de IO en %d", time_io);
	
	asignar_parametros_que_cambian_en_tiempo_de_ejecucion(config);
	config_destroy(config);
	
	log_info(logger, "Configuracion cargada correctamente de %s", path_archivo_configuracion);
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
	log_info(logger, "Algoritmo de planificacion del STS: %s", string_algoritmo);
}

void asignar_lista_lap(t_config * config){

	int prioridad;
	t_sync_queue * ptr_cola_sincronizada;

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LPN");
	prioridad = config_get_int_value(config,"PRI_LPN");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);
	log_info(logger, "Prioridad LPNuevos: %d", prioridad);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LPR");
	prioridad = config_get_int_value(config,"PRI_LPR");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);
	log_info(logger, "Prioridad LPReanudados: %d", prioridad);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LFinQ");
	prioridad = config_get_int_value(config,"PRI_LFINQ");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);
	log_info(logger, "Prioridad LPFin de Quantum: %d", prioridad);

	ptr_cola_sincronizada = dictionary_get(diccionario_colas,"LFinIO");
	prioridad = config_get_int_value(config,"PRI_LFINIO");
	list_replace(lista_auxiliar_prioridades,prioridad - 1,ptr_cola_sincronizada);
	log_info(logger, "Prioridad LPFin de IO: %d", prioridad);

}
