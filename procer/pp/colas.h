#ifndef COLAS_PROCESOS_H
#define	COLAS_PROCESOS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "commons/collections/sync_queue.h"
#include "commons/collections/dictionary.h"

extern t_sync_queue *cola_pendientes_nuevos;
extern t_sync_queue *cola_nuevos;
extern t_sync_queue *cola_pendientes_reanudar;
extern t_sync_queue *cola_reanudar;
extern t_sync_queue *cola_listos;
extern t_sync_queue *cola_fin_quantum;
extern t_sync_queue *cola_bloqueados;
extern t_sync_queue *cola_fin_bloqueados;
extern t_dictionary *tabla_suspendidos;
extern t_sync_queue *cola_fin_programa;
extern t_dictionary *tabla_procesos;
extern sem_t * mmp;
extern sem_t * semaforo_iot;
extern t_list *lista_auxiliar_prioridades;
extern bool (* algoritmo_ordenamiento)(void *, void *);
extern sem_t *mps;
extern int quantum;

void colas_initialize(void);

void cargar_lista_auxiliar_prioridades(void);

void *sacasaca(void *nada);

#ifdef	__cplusplus
}
#endif

#endif	/* COLAS_PROCESOS_H */
