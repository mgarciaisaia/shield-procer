/*
 * prueba_listas.h
 *
 *  Created on: 26/11/2012
 *      Author: utnso
 */

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

typedef struct {
	t_list * lista;
	pthread_mutex_t * sem_mutex;
} t_reg_lista;

void inicializar_listas(void);
void inicializar_diccionario(void);
void destruir_listas(void);
void destruir_diccionario(void);
void cargar_listas_en_diccionario(void);
void inicializar_lista_auxiliar_prioridades(void);
void cargar_listas_a_LAP(void);
void encolar_en_listos(void);
void encolar_lap_en_ll(void *);

int sts(void);
int procer(void);
int lts(void);

t_pcb * sacar_proceso(t_list *, pthread_mutex_t *);
void agregar_proceso(t_list *, pthread_mutex_t *, t_pcb *);
