#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "network.h"
#include <sys/stat.h>
#include <sys/sendfile.h>

struct sockaddr_in *socket_address(in_addr_t ip, uint16_t port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    memset(address, '\0', sizeof(struct sockaddr_in));
    address->sin_addr.s_addr = ip;
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    return address;
}

int socket_binded(uint16_t port) {
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socketDescriptor < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in *address = socket_address(INADDR_ANY, port);
    if(bind(socketDescriptor, (struct sockaddr*) address,
            sizeof(struct sockaddr_in))) {
        perror("bind");
        close(socketDescriptor);
        return -1;
    }
    free(address);
    return socketDescriptor;
}

/**
 * Sends the first <code>lenght</code> bytes from <code>buffer</code> via
 * <code>socket</code>, preceded by the <code>lenght</code>
 * @return buffer's sent bytes, or -1 if error
 */
int socket_send(int socket, void *buffer, uint32_t lenght) {
    int outputLenght = lenght + sizeof(lenght);
    void *output = malloc(outputLenght);
    memcpy(output, &lenght, sizeof(uint32_t));
    memcpy(output + sizeof(lenght), buffer, lenght);
    int sentChunk, sentBytes = 0;
    while(sentBytes < outputLenght ) {
        sentChunk = send(socket, output + sentBytes, outputLenght - sentBytes, 0);
        if(sentChunk < 0) {
            perror("Error sending chunk");
            free(output);
            return -1;
        }
        sentBytes += sentChunk;
    }
    free(output);
    return sentBytes - sizeof(lenght);
}

/**
 * Receives a message from <code>socket</code> and puts it in a <i>newly allocated</i>
 * memory block pointed by <code>buffer*</code>.
 * 
 * On error, allocated memory is <code>free</code>'d.
 * 
 * @param buffer a pointer to the pointer in which to point to the new buffer
 * @return received message lenght, or -1 if error
 */
int socket_receive(int socket, void **buffer) {
    int headerSize = sizeof(uint32_t);
    uint32_t messageLenght = 0;
    int receivedHeaderLenght = 0, receivedBytes = 0;
    
    while (receivedHeaderLenght < headerSize ) {
        int received = recv(socket, (&messageLenght) + receivedHeaderLenght, headerSize - receivedHeaderLenght, 0);
        if (received == 0) {
            // Connection closed
            return 0;
        } else if (received < 0) {
            perror("Error receiving header");
            return -1;
        }
        receivedHeaderLenght += received;
    }
    
    *buffer = malloc(messageLenght);
    while(receivedBytes < messageLenght) {
        int received = recv(socket, *buffer + receivedBytes, messageLenght - receivedBytes, 0);
        if(received == 0) {
            // Connection closed
            free(*buffer);
            return 0;
        } else if(received < 0) {
            perror("Error receiving chunk");
            free(*buffer);
            return -1;
        }
        receivedBytes += received;
    }
    
    return receivedBytes;
}

int socket_sendfile(int socket, int file_descriptor) {
	struct stat st;
	int sentBytes;

	if (fstat(file_descriptor, &st)) {
		perror("fstat");
		return -2;
	}
	uint32_t file_lenght = st.st_size;

	if((sentBytes = send(socket, &file_lenght, sizeof(file_lenght), 0)) <= 0) {
		if(sentBytes < 0) {
            perror("sendfile: Error sending header");
            return -3;
        } else if(sentBytes == 0) {
			printf("sendfile: Error sending header - closed connection");
			return 0;
		}
	}

	return sendfile(socket, file_descriptor, NULL, file_lenght);
}
