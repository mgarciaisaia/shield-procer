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
extern t_sync_queue *cola_suspendidos;
extern t_dictionary *tabla_procesos;
extern sem_t * mmp;


void colas_initialize(void);


void *sacasaca(void *nada);

#ifdef	__cplusplus
}
#endif

#endif	/* COLAS_PROCESOS_H */
