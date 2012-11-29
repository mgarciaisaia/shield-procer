/*
 * parser.h
 *
 *  Created on: 06/11/2012
 *      Author: utnso
 */

#include "commons/collections/dictionary.h"
#include <stdint.h>
#include "commons/collections/stack.h"
#include "commons/collections/list.h"
#include <stdbool.h>

extern int hayQueSuspenderProceso;

typedef struct {
	int id_proceso;
	uint32_t program_counter;
	t_dictionary * datos;
	t_dictionary * d_funciones;
	t_dictionary * d_etiquetas;
//	ATRIBUTOS PARA PODER USAR EN EL ALGORITMO SPN
	double valor_estimacion_anterior;
	uint32_t ultima_rafaga;
        uint8_t prioridad;

	t_stack *stack;
	char ** codigo;
} t_pcb;

//==================================================
//= ESTRUCTURA QUE VA A MANEJAR LA LISTA DE LISTOS =
//==================================================

typedef struct {
	t_pcb * pcb;
	double tiempo_entrada_listos;
} t_reg_listos;

typedef struct {
	uint32_t retorno;
	char * nombre_funcion;
} t_registro_stack;

typedef struct {
	int tiempo_acceso_io;
	t_pcb * pcb;
} t_registro_io;

int ejecutar(char *programa, int socketInterprete, uint8_t prioridadProceso);
int ejecutarPcb(t_pcb *programa);
void registrarSignalListener();
t_pcb *crear_pcb(char *programa, int socketInterprete, uint8_t prioridad);
t_pcb *nuevo_pcb(int id_proceso);
void inicializar_pcb(t_pcb *pcb);
void destruir_pcb(t_pcb *pcb);
void concatenar_estado_pcb(char **buffer, t_pcb *pcb);
int es_un_comentario(char *);
void cargar_variables_en_diccionario(t_dictionary *, char *);
void cargar_funciones_en_diccionario(t_dictionary *, char *, void *);
void cargar_etiquetas_en_diccionario(t_dictionary *, char *, void *);
int procesar(t_pcb * pcb);
void eliminar_estructuras(t_pcb *);
bool ejecutarInstruccion(t_pcb *);

int es_funcion(t_pcb *, char *);
int es_funcion_io(char *);
int es_fin_funcion(char *);
int es_fin_programa(char *);
int es_etiqueta(t_pcb *, char *);
int es_funcion_imprimir(char *);
int es_un_salto(char *);
int es_asignacion(char *);

void procesar_funcion(t_pcb *, char *);
void procesar_fin_funcion(t_pcb *,char *);
void procesar_funcion_imprimir(t_pcb *, char *);
void procesar_salto(t_pcb *, char *);
void procesar_asignacion(t_pcb *, char *);

bool posicionarse_proxima_instruccion_ejecutable(t_pcb *);
void imprimir(int pid, char *, uint32_t);

char *pid_string(int pid);

//=============================================
//= FUNCIONES PARA ORDENAR LA LISTA DE LISOTS =
//=============================================

bool es_primer_pcb_mas_antiguo(void *, void *);
double calcular_rafaga(double, double);
bool es_primer_pcb_de_menor_prioridad(void *, void *);
bool es_primer_pcb_de_rafaga_mas_corta(void *, void *);

bool es_io_bloqueante(char * );
int procesar_io(char*, t_pcb*, bool);
int tiempo_ejecucion_io(char *);
uint8_t es_bloqueante(char *);
void * ejecutar_io(void *);
