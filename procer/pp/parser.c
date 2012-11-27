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

#define FACTOR_AJUSTE_SPN 0.5

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

void suspender(t_pcb *pcb) {
    char *mensaje = NULL;
    // Mensaje que empieza con "1" hace que el PI pida confirmacion para
    // reanudar la ejecucion del proceso
    mensaje = strdup("1");
    
    string_concat(&mensaje, "----------------\n\n");
    string_concat(&mensaje, "ID=%d\n", pcb->id_proceso);
    string_concat(&mensaje, "PC=%d\n", pcb->program_counter);
    string_concat(&mensaje, "\n-- Estructura de codigo --\n");
    
    int indice = 0;
    
    while(pcb->codigo[indice] != NULL) {
        string_concat(&mensaje, "%s\n", pcb->codigo[indice++]);
    }
    
    string_concat(&mensaje, "----------------\n");
    string_concat(&mensaje, "\n-- Estructuras de datos --\n");
    
    // TODA la magia negra junta: inner functions!! (?!!?!?!)
    void mostrarVariableEnMensaje(char *variable, void *valor) {
        string_concat(&mensaje, "%s=%d\n", variable, *(int *)valor);
    }
    
    dictionary_iterator(pcb->datos, mostrarVariableEnMensaje);
    
    string_concat(&mensaje, "----------------\n\n");
    string_concat(&mensaje, "-- Estructura de Stack --\n");
    
    void mostrarEntradaStackEnMensaje(void *entradaStack) {
        string_concat(&mensaje, "%d,%s\n", ((t_registro_stack *)entradaStack)->retorno,
                ((t_registro_stack *) entradaStack)->nombre_funcion);
    }
    
    stack_iterate(pcb->stack, mostrarEntradaStackEnMensaje);
    
    string_concat(&mensaje, "\n----------------\n");
    
    string_concat(&mensaje, "\nProceso suspendido...\n\n");
    
    socket_send(pcb->id_proceso, mensaje, strlen(mensaje) + 1);
    
    free(mensaje);
    
    do {
        // FIXME: tengo que guardarme los bytes recibidos?
        socket_receive(pcb->id_proceso, (void *)&mensaje);
    } while (mensaje != NULL && strcmp(mensaje, "1REANUDARPROCESO") != 0);
}

int ejecutar(char *programa, int socketInterprete, uint8_t prioridadProceso) {

//	char * programa =
//			"#!/home/utnso/pi\n# Comentario\nvariables a\ncomienzo_programa\na=-1\nimprimir a\nfin_programa\n";
//			"#!/home/utnso/pi\n# Comentario\nvariables a\ncomienzo_programa\na=-1\nimprimir a\na=-3+a\nimprimir a\nfin_programa\n";
//			"#!/home/utnso/pi\n# Comentario\nvariables a\ncomienzo_programa\n\ta=-1\n\tf1()\n\timprimir a\nfin_programa\ncomienzo_funcion f1\n\ta=-3-a\n\tf2()\nfin_funcion f1\ncomienzo_funcion f2\n\ta=a-0\nfin_funcion";
//			"#!/home/utnso/pi\n# Comentario\nvariables i\ncomienzo_programa\n\ta=4\n\ti=3\n\tinicio_for:\n\ti=i-1\n\ta=a+2\n\tsnc i inicio_for\n\timprimir i\n\timprimir a\nfin_programa";
//			"#!/home/utnso/pi\n\n# Comentario\n\nvariables i,b\n\ncomienzo_programa\n\ti=1\n\tinicio_for:\n\ti=i+1\n\timprimir i\n\tb=i-10\n\tsnc b inicio_for\nfin_programa";
//			"#!/home/utnso/pi\n\n# Comentario\n\nvariables a\n\ncomienzo_programa\n\ta=1\n\ta=a+1\n\ta=a+2\n\timprimir a\nfin_programa";
//			"#!/home/utnso/pi\n\n# Comentario\n\nvariables a,b,c,d,e\n\ncomienzo_programa\n\ta=1\n\tb=2;3\n\tc=a+b\n\td=c-3\n\tf1()\n\tf2()\n\te=a+c;2\nimprimir a\nimprimir b\nimprimir c\nimprimir d\nimprimir e\nfin_programa\n\ncomienzo_funcion f1\n\ta=3\n\tf3()\n\tb=4\nfin_funcion f1\n\ncomienzo_funcion f2\n\ta=a+1\nfin_funcion f2\n\ncomienzo_funcion f3\n\tc=d\nfin_funcion f3";
	t_pcb * pcb = crear_pcb(programa, socketInterprete, prioridadProceso);
        registrarSignalListener();
        while(1) {
            procesar(pcb);
            if(hayQueSuspenderProceso) {
                suspender(pcb);
                hayQueSuspenderProceso = 0;
            }
        }
	eliminar_estructuras(pcb);
	return EXIT_SUCCESS;
}

/*
 * Se carga la estructura del PCB
 * Cargo los diccionarios de variables(no se setean acá!!!), funciones y etiquetas, para que al procesar
 * ya tenga las referencias
 */
t_pcb *crear_pcb(char* programa, int socketInterprete, uint8_t prioridad) {
	t_pcb *pcb = malloc(sizeof(t_pcb));
	pcb->id_proceso = socketInterprete;
        pcb->prioridad = prioridad;
	pcb->datos = dictionary_create(NULL);
	pcb->d_funciones = dictionary_create(NULL);
	pcb->d_etiquetas = dictionary_create(NULL);
        pcb->stack = stack_create();
	#define SEPARADOR_LINEAS '\n'
	pcb->codigo = string_tokens(programa, SEPARADOR_LINEAS);
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
			} else if (string_equals_ignore_case(palabra[0],
					"comienzo_funcion")) {
				cargar_funciones_en_diccionario(pcb->d_funciones, palabra[1],
						(void *) i);
			} else if ((index(palabra[0], ':')) != NULL) {
				cargar_etiquetas_en_diccionario(pcb->d_etiquetas, palabra[0],
						(void *) i);
			}
		}
		i++;
	}
        // TODO: ver si esta bueno este valor o da para cambiarlo
        pcb->valor_estimacion_anterior = i;
        pcb->ultima_rafaga = 0;
        return pcb;
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
 * Aca se van a cargar/modificar los valores de las variables, el stack, etc.
 */
// todo: ponerle las condiciones que corresponden al while
void procesar(t_pcb * pcb) {
	pcb->ultima_rafaga = 0;
	// FIXME: este sleep() es CUALQUIERA
	while(ejecutarInstruccion(pcb) && sleep(2) && !hayQueSuspenderProceso);
	printf("lineas ejecutadas por el proceso: %d\n",pcb->ultima_rafaga);
}

void eliminar_estructuras(t_pcb * pcb) {
	dictionary_destroy(pcb->datos);
	dictionary_destroy(pcb->d_funciones);
	dictionary_destroy(pcb->d_etiquetas);
	free(pcb);
}

uint32_t ejecutarInstruccion(t_pcb * pcb) {
	uint32_t valor_ejecucion = 1;
	posicionarse_proxima_instruccion_ejecutable(pcb);
	uint32_t pc = pcb->program_counter;
	char * instruccion = calloc(1,strlen(pcb->codigo[pc]) + 1);
	strncpy(instruccion,pcb->codigo[pc],strlen(pcb->codigo[pc]));
	string_trim(&instruccion);
        printf("Procesando la instruccion %d: << %s >>\n", pc, instruccion);
	if (es_fin_programa(instruccion)) {
		valor_ejecucion = 0;
	} else if (es_funcion(pcb, instruccion)) {
		procesar_funcion(pcb,instruccion);
	} else if(es_fin_funcion(instruccion)){
		procesar_fin_funcion(pcb,instruccion);
	} else if(es_funcion_imprimir(instruccion)){
		procesar_funcion_imprimir(pcb,instruccion);
	} else if(es_un_salto(instruccion)){
		procesar_salto(pcb,instruccion);
	} else if(es_funcion_io(instruccion)){
		printf("es una función io");
	} else if(es_asignacion(instruccion)){
		procesar_asignacion(pcb,instruccion);
	}
	free(instruccion);
	(pcb->program_counter)++;
	return valor_ejecucion;
}

int es_funcion(t_pcb * pcb, char * instruccion) {
	return dictionary_has_key(pcb->d_funciones, instruccion);
}

int es_etiqueta(t_pcb * pcb, char * instruccion) {
	return dictionary_has_key(pcb->d_etiquetas, instruccion);
}

int es_fin_programa(char * instruccion){
	return string_equals_ignore_case(instruccion, "fin_programa");
}

int es_fin_funcion(char * instruccion){
	return string_starts_with(instruccion,"fin_funcion");
}

int es_asignacion(char * instruccion){
	return index(instruccion,'=') != NULL;
}

int es_funcion_io(char * instruccion){
	return string_starts_with(instruccion,"io");
}

int es_funcion_imprimir(char * instruccion){
	return string_starts_with(instruccion,"imprimir");
}

int es_un_salto(char * instruccion){
	return string_starts_with(instruccion,"ssc") || string_starts_with(instruccion,"snc");
}

void procesar_funcion(t_pcb * pcb, char * instruccion){
	t_registro_stack *registro_stack = malloc(sizeof(t_registro_stack));
	registro_stack->nombre_funcion = calloc(1,strlen(instruccion) + 1);
	registro_stack->retorno=pcb->program_counter;
	strncpy(registro_stack->nombre_funcion,instruccion,strlen(instruccion));
        stack_push(pcb->stack, registro_stack);
	pcb->program_counter = (uint32_t)dictionary_get(pcb->d_funciones,instruccion);
}

void procesar_fin_funcion(t_pcb * pcb,char * instruccion){
	t_registro_stack *registro_stack = (t_registro_stack *) stack_pop(pcb->stack);
	pcb->program_counter = registro_stack->retorno;
	free(registro_stack->nombre_funcion);
}

void procesar_funcion_imprimir(t_pcb * pcb,char * instruccion){
	char ** palabra = string_split(instruccion," ");
	(pcb->ultima_rafaga)++;
	uint32_t valor_variable = (uint32_t) dictionary_get(pcb->datos,palabra[1]);
	imprimir(pcb->id_proceso, palabra[1], valor_variable);
}

void procesar_salto(t_pcb * pcb, char * instruccion){
	char ** palabra = string_split(instruccion," ");
	uint32_t valor_variable = (uint32_t)dictionary_get(pcb->datos,palabra[1]);
	if(string_equals_ignore_case(palabra[0],"ssc")){
		if(valor_variable == 0){
			string_append(&palabra[2],":");
			uint32_t dir_etiqueta = (uint32_t)dictionary_get(pcb->d_etiquetas,palabra[2]);
			pcb->program_counter = dir_etiqueta;
		}
	} else {
		if(valor_variable != 0){
			string_append(&palabra[2],":");
			uint32_t dir_etiqueta = (uint32_t)dictionary_get(pcb->d_etiquetas,palabra[2]);
			pcb->program_counter = dir_etiqueta;
		}
	}
}

void procesar_asignacion(t_pcb * pcb, char * instruccion){
	// todo: usar tiempo_ejecucion para cuando consuma el quantum
		(pcb->ultima_rafaga)++;
		char ** palabra = string_split(instruccion,"=");
		char * valor_l = palabra[0];
		char * sentencia = palabra[1];
//			char * tiempo_ejecucion = "NULL";
		if(index(instruccion,';') != NULL){
			char ** derecha_asignacion = string_split(palabra[1],";");
//				tiempo_ejecucion = derecha_asignacion[1];
			sentencia = derecha_asignacion[0];
		}
		int32_t operando = 1;
		if(string_starts_with(sentencia,"-")){
			operando = -1;
			sentencia = &sentencia[1];
		}
		if((index(sentencia,'+') != NULL) || (index(sentencia,'-') != NULL)){
			// todo: Encapsular en función, igual comportamiento
			char * separador = (index(sentencia,'+') != NULL) ? "+" : "-";
			long int valor_variable_0;
			long int valor_variable_1;
			char ** variable = string_split(sentencia,separador);
			if(dictionary_has_key(pcb->datos,variable[0])){
				valor_variable_0 = (long int)dictionary_get(pcb->datos,variable[0]);
			} else {
				valor_variable_0 = strtol(variable[0],NULL,0);
			}
			if(dictionary_has_key(pcb->datos,variable[1])){
				valor_variable_1 = (long int)dictionary_get(pcb->datos,variable[1]);
			} else {
				valor_variable_1 = strtol(variable[1],NULL,0);
			}
			long int resultado_operacion = (string_equals_ignore_case(separador,"+")) ?
					(valor_variable_0 * operando) + valor_variable_1 :
					(valor_variable_0 * operando) - valor_variable_1;
			dictionary_remove(pcb->datos,valor_l);
			dictionary_put(pcb->datos,strdup(valor_l),(void *)resultado_operacion);
		} else if(es_funcion_io(sentencia)){
// todo: ejecutar la sentencia io, devolver algo para que deje de ejecutar y pase a otro PCB
			printf("es una io");
//				char ** sentencia_io_spliteado = string_split(sentencia,",");
//				char * texto_parametro_1_io = (index(sentencia_io_spliteado[0],'('))[1];
//				char * texto_parametro_2_io =  string_split(sentencia_io_spliteado[1],")")[0];
			// pasar los parámetros para la función	io;
		} else {
			//  La sentencia es de una asignación simple de entero (ej. a=1)
			long int valor = strtol(sentencia,NULL,0) * operando;
			dictionary_remove(pcb->datos,valor_l);
			dictionary_put(pcb->datos,strdup(valor_l),(void *)valor);
		}
}

/*
 * Salta instrucciones hasta encontrar una que no sea ni un comentario, ni una etiqueta
 */
void posicionarse_proxima_instruccion_ejecutable(t_pcb * pcb) {
	while(es_un_comentario(pcb->codigo[pcb->program_counter])
			|| es_etiqueta(pcb,pcb->codigo[pcb->program_counter]))
		(pcb->program_counter)++;
}

void imprimir(int pid, char * variable, uint32_t valor) {
    char *mensaje = NULL;
    // Si el primer caracter del mensaje es '1', se pide confirmacion al usuario
    // para reanudar un proceso suspendido
    int longitudMensaje = asprintf(&mensaje, "0IMPRIMIENDO VARIABLE %s: %d", variable, valor);
    // El + 1 es porque asprintf() devuelve el strlen(); falta el \0 final
    socket_send(pid, mensaje, longitudMensaje + 1);
    free(mensaje);
}


double calcular_rafaga(double valor_estimacion_anterior, double ultima_rafaga) {
	return FACTOR_AJUSTE_SPN * ultima_rafaga + (1 - FACTOR_AJUSTE_SPN) * valor_estimacion_anterior;
}

int es_primer_pcb_mas_antiguo(t_reg_listos * reg_1, t_reg_listos * reg_2){
	return reg_1->tiempo_entrada_listos < reg_2->tiempo_entrada_listos;
}

int es_primer_pcb_de_mayor_prioridad(t_reg_listos * reg_1, t_reg_listos * reg_2){
	return reg_1->pcb->prioridad < reg_2->pcb->prioridad;
}

int es_primer_pcb_de_rafaga_mas_corta(t_reg_listos * reg_1, t_reg_listos * reg_2){
	double estimacion_rafaga_reg_1 = calcular_rafaga(reg_1->pcb->valor_estimacion_anterior,
												reg_1->pcb->ultima_rafaga);
	double estimacion_rafaga_reg_2 = calcular_rafaga(reg_2->pcb->valor_estimacion_anterior,
												reg_2->pcb->ultima_rafaga);
	return estimacion_rafaga_reg_1 < estimacion_rafaga_reg_2;
}
