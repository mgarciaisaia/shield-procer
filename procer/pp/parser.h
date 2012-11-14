/*
 * parser.h
 *
 *  Created on: 06/11/2012
 *      Author: utnso
 */

#include "collections/dictionary.h"
#include <stdint.h>
#include "collections/pila.h"
#include "collections/list.h"

typedef struct {
	uint32_t id_proceso;
	uint32_t program_counter;
	t_dictionary * datos;
	t_dictionary * d_funciones;
	t_dictionary * d_etiquetas;
//	ATRIBUTOS PARA PODER USAR EN EL ALGORITMO SPN
	double factor_ajuste;
	double valor_estimacion_anterior;
	uint32_t ultima_rafaga;

	ptrPila stack;
	char ** codigo;
} t_pcb;

//==================================================
//= ESTRUCTURA QUE VA A MANEJAR LA LISTA DE LISTOS =
//==================================================

typedef struct {
	t_pcb * pcb;
	double tiempo_entrada_listos;
	uint32_t prioridad;
} t_reg_prueba;

void cargar_estructuras(char * , t_pcb *);
int es_un_comentario(char *);
void cargar_variables_en_diccionario(t_dictionary *, char *);
void cargar_funciones_en_diccionario(t_dictionary *, char *, void *);
void cargar_etiquetas_en_diccionario(t_dictionary *, char *, void *);
void procesar(t_pcb * pcb);
void eliminar_estructuras(t_pcb *);
uint32_t ejecutarInstruccion(t_pcb *);
int es_funcion(t_pcb *, char *);
int es_etiqueta(t_pcb *, char *);
void posicionarse_proxima_instruccion_ejecutable(t_pcb *);
void imprimir(char *, uint32_t);

//================================================================================
//= FUNCIONES PARA PEDIR ELEMENTOS DE LA PILA DE ACUERDO AL TIPO DE ORDENAMIENTO =
//================================================================================

t_reg_prueba * dame_elemento_mas_antiguo(t_list *);
t_reg_prueba * dame_elemento_mayor_prioridad(t_list *);
t_reg_prueba * dame_elemento_rafaga_mas_corta(t_list *);
double calcular_rafaga(double, double, double);
