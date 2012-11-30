#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "commons/network.h"
#include "commons/log.h"

#define ERROR_PARAMETROS 1
#define NO_PP_IP 2
#define NO_PP_PUERTO 3
#define ERROR_SENDFILE 4
#define ERROR_MPS 5

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
	struct sockaddr *remote_address = (struct sockaddr*) socket_address(inet_addr(direccionPP), atoi(puertoPP));
	if(connect(conexion, remote_address, sizeof(struct sockaddr_in))) {
		printf("Error %d conectandose a %s:%s: %s\n", errno, direccionPP, puertoPP, strerror(errno));
		exit(errno);
	}
        
        free(remote_address);
        
        uint64_t *pid = NULL;
        int bytesRecibidos = socket_receive(conexion, (void*)&pid);
        if(bytesRecibidos <= 0) {
            perror("Error recibiendo el PID del proceso");
            exit(errno);
        } else if(bytesRecibidos != sizeof(uint64_t)) {
			printf("%.*s\n", bytesRecibidos, (char *)pid);
			exit(ERROR_MPS);
		}
        
	char *nombreArchivoLog = NULL;
        asprintf(&nombreArchivoLog, "PI.[%"PRIu64"].log", *pid);
        
        t_log *log = log_create(nombreArchivoLog, "PI", 1, LOG_LEVEL_TRACE);
        free(nombreArchivoLog);
        
        log_info(log, "Ejecutando el proceso con PID %d", *pid);

	uint8_t prioridadProceso = 0;
	char *prioridadEnv = getenv("prioridad_PRI");
	if(prioridadEnv != NULL) {
		prioridadProceso = atoi(prioridadEnv);
		if(prioridadProceso < 0 || prioridadProceso > 20) {
			log_error(log, "Error leyendo la prioridad del proceso - %d no es una prioridad válida", prioridadProceso);
			prioridadProceso = 0;
		}
	}

	log_info(log, "La prioridad del proceso es %d", prioridadProceso);

	int bytesEnviados = 0;
	if((bytesEnviados = socket_send(conexion, &prioridadProceso, sizeof(prioridadProceso))) <= 0) {
		log_error(log, "Error %d enviando la prioridad del proceso al PP: %s", errno, strerror(errno));
	}
        
	int bytesLeidos = 0;
	
	if((bytesLeidos = socket_sendfile(conexion, script)) <= 0) {
		log_error(log, "Error %d enviando el archivo %s por %d: %s", errno, pathScript, conexion, strerror(errno));
		close(conexion);
		close(script);
		exit(ERROR_SENDFILE);
	} else {
		log_debug(log, "Enviados %d bytes", bytesLeidos);
	}
	
	if(close(script)) {
	       log_error(log, "Error %d cerrando el archivo del script: %s", errno, strerror(errno));
	       exit(errno);
	}
	
	char confirmar = 0;
        #define PEDIDO_REANUDAR "1REANUDARPROCESO"
        char *receiveBuffer = NULL;
	
	int exit_status = 1;
		
	while((bytesLeidos = socket_receive(conexion, (void**)&receiveBuffer)) > 0) {
		if(*receiveBuffer == '2') {
			// cerramos la conexion
			free(receiveBuffer);
			exit_status = 0;
			break;
		}
		confirmar = (*receiveBuffer == '1');
                log_info(log, "%.*s", bytesLeidos - 1, receiveBuffer + 1);
                free(receiveBuffer);
		
		if(confirmar) {
			printf("Presione ENTER para reanudar la ejecucion...\n");
			// FIXME: si apreto varios ENTER seguidos, los proximos getchar directamente consmen esas teclas.
			// Habria que limpiar el buffer o algo asi - no se como.
			system("stty cbreak -echo");
			getchar();
			system("stty cooked echo");
			socket_send(conexion, PEDIDO_REANUDAR, strlen(PEDIDO_REANUDAR) + 1);
			log_info(log, "Proceso reanudado");
		}
	}

	close(conexion);
        log_destroy(log);
        free(pid);
        return exit_status;
}
