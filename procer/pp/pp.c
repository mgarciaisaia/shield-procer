#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/network.h"
#include <errno.h>
#include <unistd.h>
#include "parser.h"
#include "server.h"
#include "colas.h"
#include <pthread.h>
#include "commons/collections/list.h"


void *pendientes_nuevos(void *nada) {
	int valor;
	while(1){
		t_pcb * pcb = sync_queue_pop(cola_pendientes_nuevos);
		sem_wait(mmp);
		sync_queue_push(cola_nuevos,pcb);
		sem_getvalue(mmp,&valor);
		//todo: loguear
		printf("agregue un pcb: %d\n", valor);
	}
	return NULL;
}

#define ERROR_BINDED 1
#define ERROR_LISTEN 2
#define ERROR_SEND_PID 3
#define ERROR_RECEIVE_PRIORITY 4
int main(void) {
    printf("Iniciado PROCER con PID %d\n", getpid());
	colas_initialize();
	
    #define THREAD_COUNT 2
	void *funciones_existentes[THREAD_COUNT] = { lts, pendientes_nuevos };
	t_list *threads = list_create();
	pthread_t *thread;
	int index;
	for(index = 0; index < THREAD_COUNT; index++) {
		thread = malloc(sizeof(pthread_t));
	
		pthread_create(thread, NULL, funciones_existentes[index], NULL);
		list_add(threads, thread);
	}
	
	void *nada;
	
	void joinear_thread(void *thread) {
		pthread_join(*(pthread_t *)thread, &nada);
		free(thread);
	}
	
	list_iterate(threads, joinear_thread);
	return 0;
    int socket = socket_binded(23456);
    if(socket < 0) {
        printf("Error %d bindeando el socket: %s\n", errno, strerror(errno));
        exit(ERROR_LISTEN);
    }
    if(listen(socket, 0)) {
        printf("Error %d escuchando en el socket %d: %s\n", errno, socket, strerror(errno));
        exit(ERROR_LISTEN);
    }
    
    struct sockaddr_in address;
    socklen_t addressLength = sizeof address;
    
    int querySocket = accept(socket, (struct sockaddr *) &address, &addressLength);
    
    int sentBytes = socket_send(querySocket, &querySocket, sizeof(querySocket));
    if(sentBytes < 0) {
        perror("Error sending process' PID");
        exit(ERROR_SEND_PID);
    }
    
    void *input = NULL;
    int inputLenght = socket_receive(querySocket, &input);
    if(inputLenght != sizeof(uint8_t)) {
	    printf("Error recibiendo la prioridad del proceso %d - recibidos %d bytes, se esperaban %zd\n", querySocket, inputLenght, sizeof(uint8_t));
	    free(input);
	    exit(ERROR_RECEIVE_PRIORITY);
    }

    uint8_t prioridadProceso = *(uint8_t *) input;
    free(input);
    printf("La prioridad del proceso %d es %d\n", querySocket, prioridadProceso);

    inputLenght = socket_receive(querySocket, &input);
    printf("Recibido: %.*s\n", inputLenght, (char *)input);
    
    ejecutar((char *)input, querySocket, prioridadProceso);
    
    close(querySocket);
    close(socket);
    
    return (EXIT_SUCCESS);
}

