#include "commons/collections/sync_queue.h"
#include "commons/collections/dictionary.h"
#include "colas.h"

t_sync_queue *cola_pendientes_nuevos;
t_sync_queue *cola_nuevos;
t_sync_queue *cola_pendientes_reanudar;
t_sync_queue *cola_reanudar;
t_sync_queue *cola_listos;
t_sync_queue *cola_fin_quantum;
t_sync_queue *cola_bloqueados;
t_sync_queue *cola_fin_bloqueados;
t_sync_queue *cola_suspendidos;
t_dictionary *tabla_procesos;

void colas_initialize() {
	cola_pendientes_nuevos = sync_queue_create();
	cola_nuevos = sync_queue_create();
	cola_pendientes_reanudar = sync_queue_create();
	cola_reanudar = sync_queue_create();
	cola_listos = sync_queue_create();
	cola_fin_quantum = sync_queue_create();
	cola_bloqueados = sync_queue_create();
	cola_fin_bloqueados = sync_queue_create();
	cola_suspendidos = sync_queue_create();

	tabla_procesos = dictionary_create(NULL);
}
