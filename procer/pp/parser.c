/*
 ============================================================================
 Name        : Parser.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#define _GNU_SOURCE
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include "commons/temporal.h"
#include "commons/string.h"
#include "commons/collections/dictionary.h"
#include <string.h>
#include "commons/collections/stack.h"
#include "commons/collections/list.h"
#include "commons/network.h"
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include "commons/collections/sync_queue.h"
#include "colas.h"
#include <inttypes.h>
#include <errno.h>
#include <semaphore.h>
#include "configuracion.h"

#define FACTOR_AJUSTE_SPN 0.5

//sem_t * threads_iot;

int hayQueSuspenderProceso = 0;

void suspenderProceso(int signal) {
	hayQueSuspenderProceso = 1;
}

void registrarSignalListener() {
	struct sigaction *handler = malloc(sizeof(struct sigaction));

	handler->sa_handler = suspenderProceso;
	sigemptyset(&(handler->sa_mask));
	handler->sa_flags = SA_RESTART;

	sigaction(SIGUSR1, handler, NULL);
	log_debug(logger, "Registro el handler de la signal SIGUSR1");
	free(handler);
}

void concatenar_estado_pcb(char **buffer, t_pcb *pcb) {
	string_concat(buffer, "----------------\n\n");
    string_concat(buffer, "ID=%d\n", pcb->id_proceso);
    string_concat(buffer, "PC=%d\n", pcb->program_counter);
    string_concat(buffer, "\n-- Estructura de codigo --\n");
    
    int indice = 0;
    
    while(pcb->codigo[indice] != NULL) {
        string_concat(buffer, "%s\n", pcb->codigo[indice++]);
    }
    
    string_concat(buffer, "----------------\n");
    string_concat(buffer, "\n-- Estructuras de datos --\n");
    
    // TODA la magia negra junta: inner functions!! (?!!?!?!)
    void mostrarVariableEnMensaje(char *variable, void *elemento) {
        string_concat(buffer, "%s=%d\n", variable, (int) ((t_hash_element *)elemento)->data);
    }
    
    dictionary_iterator(pcb->datos, mostrarVariableEnMensaje);
    
    string_concat(buffer, "----------------\n\n");
    string_concat(buffer, "-- Estructura de Stack --\n");
    
    void mostrarEntradaStackEnMensaje(void *entradaStack) {
        string_concat(buffer, "%d,%s\n", ((t_registro_stack *)entradaStack)->retorno,
                ((t_registro_stack *) entradaStack)->nombre_funcion);
    }
    
    stack_iterate(pcb->stack, mostrarEntradaStackEnMensaje);
    
    string_concat(buffer, "\n----------------\n");
}

t_pcb *nuevo_pcb(int id_proceso) {
	log_trace(logger, "Creo un PCB con PID %d", id_proceso);
	t_pcb *pcb = malloc(sizeof(t_pcb));
	pcb->id_proceso = id_proceso;
	pcb->prioridad = 40;
	pcb->codigo = NULL;
	pcb->datos = dictionary_create(NULL);
	pcb->d_funciones = dictionary_create(NULL);
	pcb->d_etiquetas = dictionary_create(NULL);
	pcb->stack = stack_create();
	pcb->ultima_rafaga = 0;
	return pcb;
}

void inicializar_pcb(t_pcb *pcb) {
	int i = 0;
	log_debug(logger, "Inicializo el PCB %d", pcb->id_proceso);
	//primer recorrido para cargar estructuras del PCB
	//ID, PC, DATOS, STACK, CODIGO -- DICCIONARIO DE FUNCIONES, DICCIONARIO DE ETIQUETAS
	while (pcb->codigo[i] != NULL) {
		string_trim(&pcb->codigo[i]);
		if (!es_un_comentario(pcb->codigo[i])) {
#define SEPARADOR_PALABRAS ' '
			char ** palabra = string_tokens(pcb->codigo[i], SEPARADOR_PALABRAS);
			if (string_equals_ignore_case(palabra[0], "variables")) {
				log_trace(logger, "La linea %d declara las variables: <<%s>>", i, pcb->codigo[i]);
				cargar_variables_en_diccionario(pcb->datos, palabra[1]);
			} else if (string_equals_ignore_case(palabra[0],
					"comienzo_programa")) {
				pcb->program_counter = i + 1;
				log_debug(logger, "El programa %d comienza en la linea %d",
						pcb->id_proceso, pcb->program_counter + 1);
			} else if (string_equals_ignore_case(palabra[0],
					"comienzo_funcion")) {
				cargar_funciones_en_diccionario(pcb->d_funciones, palabra[1],
						(void *) i);
				log_trace(logger, "La funcion %s empieza en la linea %d", palabra[1], i);
			} else if ((index(palabra[0], ':')) != NULL) {
				cargar_etiquetas_en_diccionario(pcb->d_etiquetas, palabra[0],
						(void *) i);
				log_trace(logger, "La etiqueta %s esta en la linea %d", palabra[0], i);
			}
		}
		i++;
	}
	posicionarse_proxima_instruccion_ejecutable(pcb);
	log_debug(logger, "La primer instruccion ejecutable de %d es la %"PRIu32,
			pcb->id_proceso, pcb->program_counter);
	pcb->valor_estimacion_anterior = i;
}

void destruir_pcb(t_pcb *pcb) {
	log_trace(logger, "Destruyo el PCB %d", pcb->id_proceso);
	dictionary_destroy(pcb->datos);
	dictionary_destroy(pcb->d_funciones);
	dictionary_destroy(pcb->d_etiquetas);
	stack_destroy(pcb->stack);
	free(pcb->codigo);
	free(pcb);
}

char *pid_string(int pid) {
	char *pidString;
	asprintf(&pidString, "%d", pid);
	return pidString;
}

int es_un_comentario(char * linea) {
	return linea[0] == '#';
}

void cargar_variables_en_diccionario(t_dictionary * diccionario,
		char * string_variables) {
	char * separador_variables = ",";
	char ** variable = string_split(string_variables, separador_variables);
	int i;
	for (i = 0; variable[i] != NULL; i++) {
		dictionary_put(diccionario, strdup(variable[i]), 0);
		free(variable[i]);
	}
	free(variable);
}

void cargar_funciones_en_diccionario(t_dictionary * diccionario,
		char * nombre_funcion, void * nro_linea) {
	char * parentesis = "()";
	char *prototipo_funcion = strdup(nombre_funcion);
	string_append(&prototipo_funcion, parentesis);
	dictionary_put(diccionario, prototipo_funcion, nro_linea);
}

void cargar_etiquetas_en_diccionario(t_dictionary * diccionario,
		char * nombre_etiqueta, void * nro_linea) {
	dictionary_put(diccionario, strdup(nombre_etiqueta), nro_linea);
}

/*
 * Procesa una instruccion de entrada/salida y devuelve 1 si pudo procesarla
 * (es decir, el proceso paso a bloqueado o entrada/salida), o 0 si no pudo
 */
int procesar_io(char* instruccion, t_pcb* pcb) {
	// cargar los parametros que se encuentran en l ainstruccion
	int tiempo_acceso = tiempo_ejecucion_io(instruccion);
	if (es_bloqueante(instruccion)) {
		t_registro_io * registro_io = malloc(sizeof(t_registro_io));
		registro_io->tiempo_acceso_io = tiempo_acceso;
		registro_io->pcb = pcb;
		sync_queue_push(cola_bloqueados, registro_io);
		log_lsch(logger, "Mando a bloqueados a %d para una IO de %d", pcb->id_proceso, tiempo_acceso);
	} else {
		if(sem_trywait(threads_iot)){
		//  devolver codigo error y seguir ejecutando
			log_info(logger, "No se pudo realizar la entrada/salida no bloqueante");
			return 0;
		} else{
			int cantidad_hilos_io;
			sem_getvalue(threads_iot, &cantidad_hilos_io);
			log_lsch(logger, "Mando a IO no bloqueante a %d. Cantidad hilos disponibles: %d",
					pcb->id_proceso, cantidad_hilos_io);
			t_registro_io * registro_io = malloc(sizeof(t_registro_io));
			registro_io->tiempo_acceso_io = tiempo_acceso;
			registro_io->pcb = pcb;
			sync_queue_push(cola_io, registro_io);
		}
	}
	return 1;
}

/*
 * Devuelve false si el proceso fue movido a otra cola
 */
bool ejecutarInstruccion(t_pcb * pcb) {
	bool seguir_ejecutando = true;
	uint32_t pc = pcb->program_counter;
	char * instruccion = calloc(1, strlen(pcb->codigo[pc]) + 1);
	strncpy(instruccion, pcb->codigo[pc], strlen(pcb->codigo[pc]));
	string_trim(&instruccion);
	log_debug(logger, "Procesando la instruccion %d de %d: << %s >>", pc, pcb->id_proceso, instruccion);
	if (es_funcion(pcb, instruccion)) {
		log_error(logger, "No deberia ejecutar la funcion como instruccion");
		procesar_funcion(pcb, instruccion);
		sleep(time_sleep);
	} else if (es_fin_funcion(instruccion)) {
		procesar_fin_funcion(pcb, instruccion);
	} else if (es_funcion_imprimir(instruccion)) {
		procesar_funcion_imprimir(pcb, instruccion);
		sleep(time_sleep);
	} else if (es_un_salto(instruccion)) {
		procesar_salto(pcb, instruccion);
	} else if (es_funcion_io(instruccion)) {
		// cargar los parametros que se encuentran en l ainstruccion
		if(procesar_io(instruccion, pcb)){
			seguir_ejecutando = false;
		}
		sleep(time_sleep);
	} else if (es_asignacion(instruccion)) {
		if(!procesar_asignacion(pcb, instruccion)){
			seguir_ejecutando = false;
		}
	}
	free(instruccion);
	(pcb->program_counter)++;
	bool programa_continua = posicionarse_proxima_instruccion_ejecutable(pcb);
	int id_proceso = pcb->id_proceso;
	if(!programa_continua){
		int cantidad_mmp;
		log_lsch(logger, "Finalizo la ejecucion de %d, lo envio a finalizados",
				id_proceso);
		sync_queue_push(cola_fin_programa,pcb);
		sem_post(mmp);
		sem_getvalue(mmp, &cantidad_mmp);
		log_debug(logger, "Finalice %d, MMP queda en %d", id_proceso, cantidad_mmp);
	}
	seguir_ejecutando = programa_continua && seguir_ejecutando;
	log_debug(logger, "%s ejecutando el proceso %d", seguir_ejecutando ? "Sigo" : "No sigo", id_proceso);
	return seguir_ejecutando;
}

int es_funcion(t_pcb * pcb, char * instruccion) {
	return dictionary_has_key(pcb->d_funciones, instruccion);
}

int es_etiqueta(t_pcb * pcb, char * instruccion) {
	return dictionary_has_key(pcb->d_etiquetas, instruccion);
}

int es_fin_programa(char * instruccion) {
	return string_equals_ignore_case(instruccion, "fin_programa");
}

int es_fin_funcion(char * instruccion) {
	return string_starts_with(instruccion, "fin_funcion");
}

int es_asignacion(char * instruccion) {
	return index(instruccion, '=') != NULL;
}

int es_funcion_io(char * instruccion) {
	return string_starts_with(instruccion, "io");
}

int es_funcion_imprimir(char * instruccion) {
	return string_starts_with(instruccion, "imprimir");
}

int es_un_salto(char * instruccion) {
	return string_starts_with(instruccion, "ssc")
			|| string_starts_with(instruccion, "snc");
}

void procesar_funcion(t_pcb * pcb, char * instruccion) {
	t_registro_stack *registro_stack = malloc(sizeof(t_registro_stack));
	registro_stack->nombre_funcion = calloc(1, strlen(instruccion) + 1);
	registro_stack->retorno = pcb->program_counter;
	strncpy(registro_stack->nombre_funcion, instruccion, strlen(instruccion));
	stack_push(pcb->stack, registro_stack);
	pcb->program_counter = (uint32_t) dictionary_get(pcb->d_funciones,
			instruccion);
	log_trace(logger, "En %d, salto a la funcion %s (linea %d)",
			registro_stack->retorno, instruccion, pcb->program_counter);
}

void procesar_fin_funcion(t_pcb * pcb, char * instruccion) {
	t_registro_stack *registro_stack = (t_registro_stack *) stack_pop(
			pcb->stack);
	log_trace(logger, "En la linea %d, salgo de %s y retorno a la linea %d",
			pcb->program_counter, registro_stack->nombre_funcion,
			registro_stack->retorno);
	pcb->program_counter = registro_stack->retorno;
	free(registro_stack->nombre_funcion);
	free(registro_stack);
}

void procesar_funcion_imprimir(t_pcb * pcb, char * instruccion) {
	char ** palabra = string_split(instruccion, " ");
	int valor_variable = (int) dictionary_get(pcb->datos, palabra[1]);
	imprimir(pcb->id_proceso, palabra[1], valor_variable);
	log_debug(logger, "Imprimo la variable %s: %d", palabra[1], valor_variable);
}

void procesar_salto(t_pcb * pcb, char * instruccion) {
	char *retardo = strchr(instruccion, ';');
	char *sin_retardo = strndup(instruccion, retardo - instruccion);

	char ** palabra = string_split(sin_retardo, " ");
	int valor_variable = (int) dictionary_get(pcb->datos, palabra[1]);
	if (string_equals_ignore_case(palabra[0], "ssc")) {
		log_trace(logger, "Proceso un salto ssc para la variable %s = %d",
				palabra[1], valor_variable);
		if (valor_variable == 0) {
			string_append(&palabra[2], ":");
			uint32_t dir_etiqueta = (uint32_t) dictionary_get(pcb->d_etiquetas,
					palabra[2]);
			log_trace(logger, "En %d, salto a la etiqueta %s (linea %d) por ejecutar %s (%s = %d)",
					pcb->program_counter, palabra[2], dir_etiqueta, sin_retardo,
					palabra[1], valor_variable);
			pcb->program_counter = dir_etiqueta;
		}
	} else {
		if (valor_variable != 0) {
			string_append(&palabra[2], ":");
			uint32_t dir_etiqueta = (uint32_t) dictionary_get(pcb->d_etiquetas,
					palabra[2]);
			log_trace(logger, "En %d, salto a la etiqueta %s (linea %d) por ejecutar %s (%s = %d)",
					pcb->program_counter, palabra[2], dir_etiqueta, sin_retardo,
					palabra[1], valor_variable);
			pcb->program_counter = dir_etiqueta;
		}
	}
	
	if(retardo != NULL) {
		log_debug(logger, "Duermo %d segundos especificados en un salto", atoi(retardo + 1));
		sleep(atoi(retardo + 1));
	} else {
		log_debug(logger, "Duermo los %d segundos standar tras ejecutar un salto", time_sleep);
		sleep(time_sleep);
	}
}

/*
 * Ejecuta una asignacion y devuelve 1 si debe seguir ejecutandose este proceso
 * o 0 si el proceso fue a entrada/salida
 */
int procesar_asignacion(t_pcb * pcb, char * instruccion) {
	char *retardo = strchr(instruccion, ';');
	char *sentencia = strndup(instruccion, retardo - instruccion);

	char ** subexpresiones = string_split(sentencia, "=");
	char * variable_asignada = subexpresiones[0];
	char * expresion = subexpresiones[1];

	log_trace(logger, "Proceso una asignacion de %d: <<%s>>", pcb->id_proceso, expresion);

	if (es_funcion_io(expresion)) {
#define IO_OK 1
#define IO_FAIL 0
		log_trace(logger, "En %d es una asignacion de IO", pcb->id_proceso);
			if(es_bloqueante(expresion)){
				int tiempo_acceso = tiempo_ejecucion_io(expresion);
				dictionary_remove(pcb->datos, variable_asignada);
				dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_OK);
				log_debug(logger, "Asigno %d a la variable %s de %d", IO_OK, variable_asignada, pcb->id_proceso);

				t_registro_io * registro_io = malloc(sizeof(t_registro_io));
				registro_io->tiempo_acceso_io = tiempo_acceso;
				registro_io->pcb = pcb;
				sync_queue_push(cola_bloqueados, registro_io);
				log_lsch(logger, "Mando a bloqueados el proceso %d", pcb->id_proceso);
				return 0;
			} else {
				log_trace(logger, "Intento asignar una IO no bloqueante en %d", pcb->id_proceso);
				if(sem_trywait(threads_iot)){
				//  devolver codigo error y seguir ejecutando
					log_warning(logger, "No pude bloquear un thread de IO para %d", pcb->id_proceso);
					dictionary_remove(pcb->datos, variable_asignada);
					dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_FAIL);
					log_debug(logger, "Asigno %d a la variable %s de %d", IO_FAIL, variable_asignada, pcb->id_proceso);
					return 1;
				} else{
					dictionary_remove(pcb->datos, variable_asignada);
					dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_OK);
					log_debug(logger, "Asigno %d a la variable %s de %d", IO_OK, variable_asignada, pcb->id_proceso);

					int tiempo_acceso = tiempo_ejecucion_io(expresion);
					int cantidad_hilos_io;
					sem_getvalue(threads_iot, &cantidad_hilos_io);
					log_lsch(logger, "Mando a IO no bloqueante a %d. Cantidad hilos disponibles: %d",
						pcb->id_proceso, cantidad_hilos_io);
					t_registro_io * registro_io = malloc(sizeof(t_registro_io));
					registro_io->tiempo_acceso_io = tiempo_acceso;
					registro_io->pcb = pcb;
					sync_queue_push(cola_io, registro_io);
					return 0;
				}

			}
	} else {
		
		int32_t operando = 1;
		if (string_starts_with(expresion, "-")) {
			operando = -1;
			expresion = &expresion[1];
		}
		
		if ((index(expresion, '+') != NULL) || (index(expresion, '-') != NULL)) {
			char * separador = (index(expresion, '+') != NULL) ? "+" : "-";
			long int valor_variable_0;
			long int valor_variable_1;
			char ** variable = string_split(expresion, separador);
			if (dictionary_has_key(pcb->datos, variable[0])) {
				valor_variable_0 = (long int) dictionary_get(pcb->datos,
						variable[0]);
			} else {
				valor_variable_0 = strtol(variable[0], NULL, 0);
			}
			if (dictionary_has_key(pcb->datos, variable[1])) {
				valor_variable_1 = (long int) dictionary_get(pcb->datos,
						variable[1]);
			} else {
				valor_variable_1 = strtol(variable[1], NULL, 0);
			}
			long int resultado_operacion =
					(string_equals_ignore_case(separador, "+")) ?
							(valor_variable_0 * operando) + valor_variable_1 :
							(valor_variable_0 * operando) - valor_variable_1;
			dictionary_remove(pcb->datos, variable_asignada);
			dictionary_put(pcb->datos, strdup(variable_asignada),
					(void *) resultado_operacion);
		} else {
			//  La sentencia es de una asignación simple de entero (ej. a=1)
			long int valor = strtol(expresion, NULL, 0) * operando;
			dictionary_remove(pcb->datos, variable_asignada);
			dictionary_put(pcb->datos, strdup(variable_asignada), (void *) valor);
		}
		
		if(retardo != NULL) {
			log_debug(logger, "Duermo %d segundos especificados en una asignacion", atoi(retardo + 1));
			sleep(atoi(retardo + 1));
		} else {
			log_debug(logger, "Duermo los %d segundos standar tras ejecutar una asignacion", time_sleep);
			sleep(time_sleep);
		}
	}
	return 1;
}

int es_linea_en_blanco(char *linea) {
	char *instruccion = strdup(linea);
	string_trim(&instruccion);
	int is_empty = string_is_empty(instruccion);
	free(instruccion);
	return is_empty;
}

/*
 * Salta instrucciones hasta encontrar una que no sea ni un comentario, ni una etiqueta
 */
bool posicionarse_proxima_instruccion_ejecutable(t_pcb * pcb) {
	while (es_un_comentario(pcb->codigo[pcb->program_counter])
			|| es_etiqueta(pcb, pcb->codigo[pcb->program_counter])
			|| es_linea_en_blanco(pcb->codigo[pcb->program_counter])) {
		log_trace(logger, "Salteo la linea %d <<%s>>", pcb->program_counter,
				pcb->codigo[pcb->program_counter]);
		(pcb->program_counter)++;
	}
	if (es_fin_programa(pcb->codigo[pcb->program_counter])) {
		log_info(logger, "Alcanzado el fin del programa en %d: <<%s>>",
				pcb->program_counter, pcb->codigo[pcb->program_counter]);
		return false;
	} else if(es_funcion(pcb, pcb->codigo[pcb->program_counter])) {
		procesar_funcion(pcb, pcb->codigo[pcb->program_counter]);
	}
	log_debug(logger, "Proxima instruccion a ejecutar de %d: <<%s>> (linea %d)",
		pcb->id_proceso, pcb->codigo[pcb->program_counter], pcb->program_counter);
	return true;
}

void imprimir(int pid, char * variable, uint32_t valor) {
	char *mensaje = NULL;
	// Si el primer caracter del mensaje es '1', se pide confirmacion al usuario
	// para reanudar un proceso suspendido
	int longitudMensaje = asprintf(&mensaje, "0IMPRIMIENDO VARIABLE %s: %d",
			variable, valor);
	// El + 1 es porque asprintf() devuelve el strlen(); falta el \0 final
	socket_send(pid, mensaje, longitudMensaje + 1);
	free(mensaje);
}

double calcular_rafaga(double valor_estimacion_anterior, double ultima_rafaga) {
	return FACTOR_AJUSTE_SPN * ultima_rafaga
			+ (1 - FACTOR_AJUSTE_SPN) * valor_estimacion_anterior;
}

bool es_primer_pcb_mas_antiguo(void * reg_void_1, void * reg_void_2) {
	t_reg_listos * reg_1 = (t_reg_listos *) reg_void_1;
	log_trace(logger, "Tiempo entrada de %d: %lf", reg_1->pcb->id_proceso,
			reg_1->tiempo_entrada_listos);
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	log_trace(logger, "Tiempo entrada de %d: %lf", reg_2->pcb->id_proceso,
			reg_2->tiempo_entrada_listos);
	return reg_1->tiempo_entrada_listos < reg_2->tiempo_entrada_listos;
}

bool es_primer_pcb_de_menor_prioridad(void * reg_void_1, void * reg_void_2) {
	t_reg_listos * reg_1 = (t_reg_listos *) reg_void_1;
	log_trace(logger, "Prioridad de %d: %d", reg_1->pcb->id_proceso,
			reg_1->pcb->prioridad);
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	log_trace(logger, "Prioridad de %d: %d", reg_2->pcb->id_proceso,
			reg_2->pcb->prioridad);
	return reg_1->pcb->prioridad < reg_2->pcb->prioridad;
}

bool es_primer_pcb_de_rafaga_mas_corta(void * reg_void_1, void * reg_void_2) {
	t_reg_listos * reg_1 = (t_reg_listos *) reg_void_1;
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	double estimacion_rafaga_reg_1 = calcular_rafaga(
			reg_1->pcb->valor_estimacion_anterior, reg_1->pcb->ultima_rafaga);
	log_trace(logger, "Estimacion anterior de %d: %lf", reg_1->pcb->id_proceso,
			reg_1->pcb->valor_estimacion_anterior);
	log_trace(logger, "Ultima rafaga de %d: %d", reg_1->pcb->id_proceso,
			reg_1->pcb->ultima_rafaga);
	log_trace(logger, "Estimacion de %d: %lf", reg_1->pcb->id_proceso,
			estimacion_rafaga_reg_1);
	double estimacion_rafaga_reg_2 = calcular_rafaga(
			reg_2->pcb->valor_estimacion_anterior, reg_2->pcb->ultima_rafaga);
	log_trace(logger, "Estimacion anterior de %d: %lf", reg_2->pcb->id_proceso,
			reg_2->pcb->valor_estimacion_anterior);
	log_trace(logger, "Ultima rafaga %d: %d", reg_2->pcb->id_proceso,
			reg_2->pcb->ultima_rafaga);
	log_trace(logger, "Estimacion de %d: %lf", reg_2->pcb->id_proceso,
			estimacion_rafaga_reg_2);
	return estimacion_rafaga_reg_1 < estimacion_rafaga_reg_2;
}

int tiempo_ejecucion_io(char * instruccion){
	char ** instruccion_spliteada = string_split(instruccion,",");
	char * primer_argumento_io = index(instruccion_spliteada[0],'(');
	primer_argumento_io++;
	uint8_t uint_primer_argumento_io = strtol(primer_argumento_io,NULL,0);
	return uint_primer_argumento_io;
}

uint8_t es_bloqueante(char * instruccion){
	char ** instruccion_spliteada = string_split(instruccion,",");
	char * segundo_argumento_io = calloc(1,strlen(instruccion_spliteada[1]));
	strncpy(segundo_argumento_io,instruccion_spliteada[1],strlen(instruccion_spliteada[1]) - 1);
	uint8_t uint_segundo_argumento_io = strtol(segundo_argumento_io,NULL,0);
	free(segundo_argumento_io);
	return uint_segundo_argumento_io;
}
