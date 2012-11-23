#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "commons/network.h"

#define ERROR_PARAMETROS 1
#define NO_PP_IP 2
#define NO_PP_PUERTO 3

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Error de parámetros\n");
		printf("** USO: %s SCRIPT_ANSISOP\n", argv[0]);
		exit(ERROR_PARAMETROS);
	}
	
	char *pathScript = argv[1];
	int script = open(pathScript, O_RDONLY);
	if(script < 0) {
		printf("Error %d abriendo el script %s: %s\n", errno, pathScript, strerror(errno));
		exit(errno);
	}
	
	char* direccionPP = getenv("PP_IP");
	if(direccionPP == NULL) {
		printf("Error - debe setearse la variable de entorno PP_IP con la IP del Proceso Planificador\n");
		printf("Ejemplo: export PP_IP=127.0.0.1\n");
		exit(NO_PP_IP);
	}
	
	char* puertoPP = getenv("PP_Puerto");
	if(puertoPP == NULL) {
		printf("Error - debe setearse la variable de entorno PP_Puerto con el puerto del Proceso Planificador\n");
		printf("Ejemplo: export PP_Puerto=4925\n");
		exit(NO_PP_PUERTO);
	}

	printf("Nos conectamos a %s:%s\n", direccionPP, puertoPP);
	
	int conexion = socket(PF_INET, SOCK_STREAM, 0);
	if(connect(conexion, (struct sockaddr*) socket_address(inet_addr(direccionPP), atoi(puertoPP)), sizeof(struct sockaddr_in))) {
		printf("Error %d conectandose a %s:%s: %s\n", errno, direccionPP, puertoPP, strerror(errno));
		exit(errno);
	}
        
        int *pid = NULL;
        int bytesRecibidos = socket_receive(conexion, &pid);
        if(bytesRecibidos <= 0) {
            perror("Error recibiendo el PID del proceso");
            exit(errno);
        }
        
        printf("El PID del proceso es %d\n", *pid);
	
	#define TAMANIO_BUFFER 1024
	char buffer[TAMANIO_BUFFER];
	int bytesLeidos = 0;
	int bytesEnviados = 0;
	
	while((bytesLeidos = read(script, &buffer, TAMANIO_BUFFER)) > 0) {
		if((bytesEnviados = socket_send(conexion, buffer, bytesLeidos)) > 0) {
			printf("Envie %d de %d bytes por %d\n", bytesEnviados, bytesLeidos, conexion);
		}
		
                printf("Lei la linea: %.*s\n", bytesLeidos - 1, buffer + 1);
	}
	
	if(close(script)) {
	       printf("Error %d cerrando el archivo del script: %s\n", errno, strerror(errno));
	       exit(errno);
	}
	
	char confirmar = 0;
	char *pedidoReanudar = "1REANUDARPROCESO";
        char *receiveBuffer = NULL;
	
	while((bytesLeidos = socket_receive(conexion, (void**)&receiveBuffer)) > 0) {
		confirmar = (*receiveBuffer == '1');
		printf("PROCER dice: %.*s\n", bytesLeidos - 1, receiveBuffer + 1);
                free(receiveBuffer);
		
		if(confirmar) {
			printf("Proceso suspendido - presione ENTER para reanudar la ejecucion...\n");
			// FIXME: si apreto varios ENTER seguidos, los proximos getchar directamente consmen esas teclas.
			// Habria que limpiar el buffer o algo asi - no se como.
			system("stty cbreak -echo");
			getchar();
			system("stty cooked echo");
			socket_send(conexion, pedidoReanudar, strlen(pedidoReanudar) + 1);
			printf("Proceso reanudado\n");
		}
	}

	close(conexion);
        return EXIT_SUCCESS;
}
