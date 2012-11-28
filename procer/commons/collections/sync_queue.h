#ifndef SYNC_QUEUE_H
#define	SYNC_QUEUE_H

#ifdef	__cplusplus
extern "C" {
#endif
	
	#include "list.h"
	#include <semaphore.h>
	#include <pthread.h>

	typedef struct {
		t_list *queue;
		sem_t *semaphore;
		pthread_mutex_t *mutex;
	} t_sync_queue;

	t_sync_queue *sync_queue_create(void);
	void sync_queue_push(t_sync_queue *, void *);
	void *sync_queue_pop(t_sync_queue *);
	void *sync_queue_peek(t_sync_queue *);
	
	void sync_queue_clean(t_sync_queue *);
	void sync_queue_destroy(t_sync_queue *);
	
	int sync_queue_size(t_sync_queue *);
	int sync_queue_is_empty(t_sync_queue *);

#ifdef	__cplusplus
}
#endif

#endif	/* SYNC_QUEUE_H */
