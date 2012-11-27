#ifndef NETWORK_H
#define	NETWORK_H

#include <netinet/in.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

struct sockaddr_in *socket_address(in_addr_t ip, uint16_t port);
int socket_binded(uint16_t port);
int socket_send(int socket, void *buffer, uint32_t lenght);
int socket_receive(int socket, void **buffer);
int socket_sendfile(int socket, int file_descriptor);


#ifdef	__cplusplus
}
#endif

#endif	/* NETWORK_H */
