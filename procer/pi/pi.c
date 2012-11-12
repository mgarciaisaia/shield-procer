#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in *socket_address(in_addr_t ip, uint16_t port);

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
	
	#define TAMANIO_BUFFER 1024
	char buffer[TAMANIO_BUFFER];
	int bytesLeidos = 0;
	int bytesEnviados = 0;
	char *otroBuffer = NULL;
	
	while((bytesLeidos = read(script, &buffer, TAMANIO_BUFFER)) > 0) {
		// TODO: manejar cuando bytesEnviados < bytesLeidos
		if(bytesEnviados = send(conexion, buffer, bytesLeidos, 0) > 0) {
			printf("Envie %d de %d bytes por %d\n", bytesEnviados, bytesLeidos, conexion);
		}
		
		otroBuffer = malloc(bytesLeidos + 1);
		memcpy(otroBuffer, buffer, bytesLeidos);
		otroBuffer[bytesLeidos] = '\0';
		printf("Lei la linea: %s\n", otroBuffer);
		free(otroBuffer);
	}
	
	if(close(script)) {
	       printf("Error %d cerrando el archivo del script: %s\n", errno, strerror(errno));
	       exit(errno);
	}
	
	char confirmar = 0;
	char *pedidoReanudar = "1REANUDARPROCESO";
	
	while((bytesLeidos = recv(conexion, &buffer, TAMANIO_BUFFER, 0)) > 0) {
		confirmar = buffer[0] == '1';
		otroBuffer = malloc(bytesLeidos);
		memcpy(otroBuffer, buffer + 1, bytesLeidos - 1);
		otroBuffer[bytesLeidos - 1] = '\0';
		printf("PROCER dice: %s\n", otroBuffer);
		free(otroBuffer);
		
		if(confirmar) {
			printf("Proceso suspendido - presione ENTER para reanudar la ejecucion...\n");
			// FIXME: si apreto varios ENTER seguidos, los proximos getchar directamente consmen esas teclas.
			// Habria que limpiar el buffer o algo asi - no se como.
			system("stty cbreak -echo");
			getchar();
			system("stty cooked echo");
			send(conexion, pedidoReanudar, strlen(pedidoReanudar) + 1, 0);
			printf("Proceso reanudado\n");
		}
	}

	close(conexion);
}

struct sockaddr_in *socket_address(in_addr_t ip, uint16_t port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    memset(address, '\0', sizeof(struct sockaddr_in));
    address->sin_addr.s_addr = ip;
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    return address;
}
