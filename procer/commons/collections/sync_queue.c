#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "list.h"
#include "sync_queue.h"

t_sync_queue *sync_queue_create(void) {
	t_sync_queue *new = malloc(sizeof(t_sync_queue));
	new->queue = list_create();
	new->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(new->mutex, NULL);
	new->semaphore = malloc(sizeof(sem_t));
	sem_init(new->semaphore, 0, 0);
	return new;
}

void sync_queue_push(t_sync_queue *self, void *element) {
	pthread_mutex_lock(self->mutex);
	list_add(self->queue, element);
	sem_post(self->semaphore);
	pthread_mutex_unlock(self->mutex);
}

void *sync_queue_pop(t_sync_queue *self) {
	void *element;
	sem_wait(self->semaphore);
	pthread_mutex_lock(self->mutex);
	element = list_remove(self->queue, 0);
	pthread_mutex_unlock(self->mutex);
	return element;
}

void *sync_queue_peek(t_sync_queue *self) {
	return list_get(self->queue, 0);
}

void sync_queue_clean(t_sync_queue *self) {
	pthread_mutex_lock(self->mutex);
	list_clean(self->queue);
	sem_destroy(self->semaphore);
	sem_init(self->semaphore, 0, 0);
	pthread_mutex_unlock(self->mutex);
}

void sync_queue_destroy(t_sync_queue *self) {
	pthread_mutex_lock(self->mutex);
	list_destroy(self->queue);
	sem_destroy(self->semaphore);
	pthread_mutex_destroy(self->mutex);
	free(self);
}
	
int sync_queue_size(t_sync_queue *self) {
	int size;
	pthread_mutex_lock(self->mutex);
	sem_getvalue(self->semaphore, &size);
	pthread_mutex_unlock(self->mutex);
	return size;
}

int sync_queue_is_empty(t_sync_queue *self) {
	return sync_queue_size(self) == 0;
}