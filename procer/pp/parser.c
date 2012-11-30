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
    void mostrarVariableEnMensaje(char *variable, void *valor) {
        string_concat(buffer, "%s=%d\n", variable, *(int *)valor);
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
	//primer recorrido para cargar estructuras del PCB
	//ID, PC, DATOS, STACK, CODIGO -- DICCIONARIO DE FUNCIONES, DICCIONARIO DE ETIQUETAS
	while (pcb->codigo[i] != NULL) {
		string_trim(&pcb->codigo[i]);
		if (!es_un_comentario(pcb->codigo[i])) {
#define SEPARADOR_PALABRAS ' '
			char ** palabra = string_tokens(pcb->codigo[i], SEPARADOR_PALABRAS);
			if (string_equals_ignore_case(palabra[0], "variables")) {
				cargar_variables_en_diccionario(pcb->datos, palabra[1]);
			} else if (string_equals_ignore_case(palabra[0],
					"comienzo_programa")) {
				pcb->program_counter = i + 1;
				printf("El programa comienza en la linea %d\n",
						pcb->program_counter);
			} else if (string_equals_ignore_case(palabra[0],
					"comienzo_funcion")) {
				cargar_funciones_en_diccionario(pcb->d_funciones, palabra[1],
						(void *) i);
				printf("La funcion %s empieza en la linea %d\n", palabra[1], i);
			} else if ((index(palabra[0], ':')) != NULL) {
				cargar_etiquetas_en_diccionario(pcb->d_etiquetas, palabra[0],
						(void *) i);
				printf("La etiqueta %s esta en la linea %d\n", palabra[0], i);
			}
		}
		i++;
	}
	posicionarse_proxima_instruccion_ejecutable(pcb);
	printf("La primer instruccion ejecutable de %d es la %"PRIu32"\n", pcb->id_proceso, pcb->program_counter);
	pcb->valor_estimacion_anterior = i;
}

void destruir_pcb(t_pcb *pcb) {
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
	}
}

void cargar_funciones_en_diccionario(t_dictionary * diccionario,
		char * nombre_funcion, void * nro_linea) {
	char * parentesis = "()";
	string_append(&nombre_funcion, parentesis);
	dictionary_put(diccionario, strdup(nombre_funcion), nro_linea);
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
		// fixme: crear estructura para las io, que guarde el tiempo
		t_registro_io * registro_io = malloc(sizeof(t_registro_io));
		registro_io->tiempo_acceso_io = tiempo_acceso;
		registro_io->pcb = pcb;
		sync_queue_push(cola_bloqueados, registro_io);
		printf("meti un io bloqueante\n");
	} else {
		if(sem_trywait(threads_iot)){
		//  devolver codigo error y seguir ejecutando
			printf("no pudo realizar el wait\n");
			return 0;
		} else{
			int cantidad_hilos_io;
			sem_getvalue(threads_iot, &cantidad_hilos_io);
			printf("Mando a IO no bloqueante a %d. Cantidad hilos disponibles: %d\n", pcb->id_proceso, cantidad_hilos_io);
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
	printf("Procesando la instruccion %d de %d: << %s >>\n", pc, pcb->id_proceso, instruccion);
	if (es_funcion(pcb, instruccion)) {
		procesar_funcion(pcb, instruccion);
	} else if (es_fin_funcion(instruccion)) {
		procesar_fin_funcion(pcb, instruccion);
	} else if (es_funcion_imprimir(instruccion)) {
		procesar_funcion_imprimir(pcb, instruccion);
	} else if (es_un_salto(instruccion)) {
		procesar_salto(pcb, instruccion);
	} else if (es_funcion_io(instruccion)) {
		// cargar los parametros que se encuentran en l ainstruccion
		if(procesar_io(instruccion, pcb)){
			seguir_ejecutando = false;
		}
	} else if (es_asignacion(instruccion)) {
		if(!procesar_asignacion(pcb, instruccion)){
			seguir_ejecutando = false;
		}
	}
	free(instruccion);
	(pcb->program_counter)++;
	bool programa_continua = posicionarse_proxima_instruccion_ejecutable(pcb);
	if(!programa_continua){
		printf("finalizo %d\n",pcb->id_proceso);
		sync_queue_push(cola_fin_programa,pcb);
		sem_post(mmp);
	}
	seguir_ejecutando = programa_continua && seguir_ejecutando;
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
	printf("Entro a la funcion %s en la linea %d\n", instruccion,
			pcb->program_counter);
	t_registro_stack *registro_stack = malloc(sizeof(t_registro_stack));
	registro_stack->nombre_funcion = calloc(1, strlen(instruccion) + 1);
	registro_stack->retorno = pcb->program_counter;
	strncpy(registro_stack->nombre_funcion, instruccion, strlen(instruccion));
	stack_push(pcb->stack, registro_stack);
	pcb->program_counter = (uint32_t) dictionary_get(pcb->d_funciones,
			instruccion);
}

void procesar_fin_funcion(t_pcb * pcb, char * instruccion) {
	printf("Salgo de la funcion %s en la linea %d\n", instruccion,
			pcb->program_counter);
	t_registro_stack *registro_stack = (t_registro_stack *) stack_pop(
			pcb->stack);
	pcb->program_counter = registro_stack->retorno;
	free(registro_stack->nombre_funcion);
}

void procesar_funcion_imprimir(t_pcb * pcb, char * instruccion) {
	char ** palabra = string_split(instruccion, " ");
	(pcb->ultima_rafaga)++;
	uint32_t valor_variable = (uint32_t) dictionary_get(pcb->datos, palabra[1]);
	imprimir(pcb->id_proceso, palabra[1], valor_variable);
}

void procesar_salto(t_pcb * pcb, char * instruccion) {
	char *retardo = strchr(instruccion, ';');
	char *sin_retardo = strndup(instruccion, retardo - instruccion);

	char ** palabra = string_split(sin_retardo, " ");
	uint32_t valor_variable = (uint32_t) dictionary_get(pcb->datos, palabra[1]);
	if (string_equals_ignore_case(palabra[0], "ssc")) {
		if (valor_variable == 0) {
			string_append(&palabra[2], ":");
			uint32_t dir_etiqueta = (uint32_t) dictionary_get(pcb->d_etiquetas,
					palabra[2]);
			pcb->program_counter = dir_etiqueta;
		}
	} else {
		if (valor_variable != 0) {
			string_append(&palabra[2], ":");
			uint32_t dir_etiqueta = (uint32_t) dictionary_get(pcb->d_etiquetas,
					palabra[2]);
			pcb->program_counter = dir_etiqueta;
		}
	}
	
	if(retardo != NULL) {
		printf("Duermo %d en un salto\n", atoi(retardo + 1));
		sleep(atoi(retardo + 1));
	}
}

/*
 * Ejecuta una asignacion y devuelve 1 si debe seguir ejecutandose este proceso
 * o 0 si el proceso fue a entrada/salida
 */
int procesar_asignacion(t_pcb * pcb, char * instruccion) {
	// todo: usar tiempo_ejecucion para cuando consuma el quantum
	(pcb->ultima_rafaga)++;
	char *retardo = strchr(instruccion, ';');
	char *sentencia = strndup(instruccion, retardo - instruccion);

	char ** subexpresiones = string_split(sentencia, "=");
	char * variable_asignada = subexpresiones[0];
	char * expresion = subexpresiones[1];


	if (es_funcion_io(expresion)) {
#define IO_OK 1
#define IO_FAIL 0
		printf("es una io\n");
			if(es_bloqueante(expresion)){
				int tiempo_acceso = tiempo_ejecucion_io(expresion);
				dictionary_remove(pcb->datos, variable_asignada);
				dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_OK);

				t_registro_io * registro_io = malloc(sizeof(t_registro_io));
				registro_io->tiempo_acceso_io = tiempo_acceso;
				registro_io->pcb = pcb;
				sync_queue_push(cola_bloqueados, registro_io);
				printf("meti un io bloqueante\n");
				return 0;
			} else {

				if(sem_trywait(threads_iot)){
				//  devolver codigo error y seguir ejecutando
					printf("no pudo realizar el wait\n");
					dictionary_remove(pcb->datos, variable_asignada);
					dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_FAIL);
					return 1;
				} else{
					dictionary_remove(pcb->datos, variable_asignada);
					dictionary_put(pcb->datos, strdup(variable_asignada), (void *) IO_OK);

					int tiempo_acceso = tiempo_ejecucion_io(expresion);
					int cantidad_hilos_io;
					sem_getvalue(threads_iot, &cantidad_hilos_io);
					printf("Mando a IO no bloqueante a %d. Cantidad hilos disponibles: %d\n", pcb->id_proceso, cantidad_hilos_io);
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
			printf("Duermo %d segundos por asignacion\n", atoi(retardo + 1));
			sleep(atoi(retardo + 1));
		}
	}
	return 1;
}

/*
 * Salta instrucciones hasta encontrar una que no sea ni un comentario, ni una etiqueta
 */
bool posicionarse_proxima_instruccion_ejecutable(t_pcb * pcb) {
	while (es_un_comentario(pcb->codigo[pcb->program_counter])
			|| es_etiqueta(pcb, pcb->codigo[pcb->program_counter]))
		(pcb->program_counter)++;
	if (es_fin_programa(pcb->codigo[pcb->program_counter])) {
		return false;
	}
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
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	return reg_1->tiempo_entrada_listos < reg_2->tiempo_entrada_listos;
}

bool es_primer_pcb_de_menor_prioridad(void * reg_void_1, void * reg_void_2) {
	t_reg_listos * reg_1 = (t_reg_listos *) reg_void_1;
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	return reg_1->pcb->prioridad < reg_2->pcb->prioridad;
}

bool es_primer_pcb_de_rafaga_mas_corta(void * reg_void_1, void * reg_void_2) {
	t_reg_listos * reg_1 = (t_reg_listos *) reg_void_1;
	t_reg_listos * reg_2 = (t_reg_listos *) reg_void_2;
	double estimacion_rafaga_reg_1 = calcular_rafaga(
			reg_1->pcb->valor_estimacion_anterior, reg_1->pcb->ultima_rafaga);
	double estimacion_rafaga_reg_2 = calcular_rafaga(
			reg_2->pcb->valor_estimacion_anterior, reg_2->pcb->ultima_rafaga);
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
