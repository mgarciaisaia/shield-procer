#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/network.h"
#include <errno.h>
#include <unistd.h>


#define ERROR_BINDED 1
#define ERROR_LISTEN 2
#define ERROR_SEND_PID 3
int main(void) {
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
    
    printf("Recibido: %.*s\n", inputLenght, (char *)input);
    
    close(querySocket);
    close(socket);
    
    return (EXIT_SUCCESS);
}
