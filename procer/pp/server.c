/*
 ** selectserver.c -- a cheezy multiperson chat server
 */
#define _GNU_SOURCE
#include "colas.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "commons/network.h"
#include "parser.h"
#include "commons/collections/dictionary.h"
#include "commons/collections/list.h"
#include "commons/collections/sync_queue.h"
#include "commons/string.h"
#include "configuracion.h"
#include <errno.h>

#define ERROR_MPS "Error - se alcanzo el maximo de procesos en el sistema (MPS)"

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void *lts(void *nada) {
	fd_set master; // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number

	int listener; // listening socket descriptor
	int newfd; // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char *buf = NULL; // buffer for client data
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1; // for setsockopt() SO_REUSEADDR, below
	int i, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	log_info(logger, "El puerto de escucha es %s", puerto_tcp);
	if ((rv = getaddrinfo(NULL, puerto_tcp, &hints, &ai)) != 0) {
		log_error(logger, "Error obteniendo la direccion local: %s", gai_strerror(rv));
		exit(1);
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		log_error(logger, "Error bindeando el servidor");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) {
		log_error(logger, "Error %d escuchando el puerto %s: %s", errno, puerto_tcp, strerror(errno));
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	for (;;) {
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error(logger, "Error %d ejecutando select: %s", errno, strerror(errno));
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener) {
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener,
							(struct sockaddr *) &remoteaddr,
							&addrlen);

					if (newfd == -1) {
						log_error(logger, "Error %d aceptando una nueva conexion: %s", errno, strerror(errno));
					} else {
						log_info(logger, "Nueva conexion de %s en %d", 
								inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*) &remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
								newfd);
						if(sem_trywait(mps)) {
							// no hay mps disponible
							socket_send(newfd, ERROR_MPS, strlen(ERROR_MPS));
							log_info(logger, "Rechazo una conexion en %d porque no tengo mas MPS", newfd);
							close(newfd);
							break;
						}
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) { // keep track of the max
							fdmax = newfd;
						}
						t_pcb *pcb = nuevo_pcb(newfd);
						char *pidS = pid_string(pcb->id_proceso);
						dictionary_put(tabla_procesos, pidS, pcb);
						uint64_t pid = newfd;
						socket_send(newfd, &pid, sizeof (pid));
					}
				} else {
					// handle data from a client
					log_trace(logger, "Nuevos datos en %d", i);
					if ((nbytes = socket_receive(i, (void **) &buf)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							// connection closed
							log_info(logger, "El cliente del proceso %d cerro la conexion", i);
						} else {
							log_error(logger, "Error %d ejecutando recv: %s", errno, strerror(errno));
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} else {
						// we got some data from a client
						log_trace(logger, "Lei esto en %d: <<%.*s>> (%d bytes)", i, nbytes, (char *) buf, nbytes);
						char *pidS = pid_string(i);
						t_pcb *pcb = dictionary_get(tabla_procesos, pidS);
						
						if(pcb->prioridad > 20) {
							pcb->prioridad = (uint8_t) *buf;
							log_debug(logger, "La prioridad del proceso %d es %d", pcb->id_proceso, pcb->prioridad);
							free(buf);
						} else if(pcb->codigo == NULL) {
							pcb->codigo = string_tokens(buf, '\n');
							inicializar_pcb(pcb);
							log_lsch(logger, "Muevo el proceso %d a la cola de pendientes_nuevos", pcb->id_proceso);
							sync_queue_push(cola_pendientes_nuevos, pcb);
						} else {
							if(strncmp("1REANUDARPROCESO", buf, strlen("1REANUDARPROCESO") + 1) == 0) {
								log_debug(logger, "Llego un pedido para reanudar el proceso %d", i);
								pcb = dictionary_remove(tabla_suspendidos, pidS);
								if(pcb == NULL) {
									log_warning(logger, "Llego un pedido de reanudar un proceso no-suspendido: %d", i);
								} else {
									log_lsch(logger, "Muevo el proceso %d a pendientes_reanudar", pcb->id_proceso);
									sync_queue_push(cola_pendientes_reanudar, pcb);
								}
							} else {
								log_warning(logger, "Mensaje desconocido: %.*s", nbytes, buf);
							}
						}
						free(pidS);
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!

	return NULL;
}

