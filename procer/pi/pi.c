#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Error de parámetros\n");
		printf("** USO: %s SCRIPT_ANSISOP\n", argv[0]);
		exit(1);
	}
	
	char *pathScript = argv[1];
	int script = open(pathScript, O_RDONLY);
	if(script < 0) {
		printf("Error %d abriendo el script %s: %s\n", errno, pathScript, strerror(errno));
		exit(errno);
	}
	
	char* direccionPP = getenv("PP_IP");
	char* puertoPP = getenv("PP_Puerto");
	
	printf("Nos conectamos a %s:%s\n", direccionPP, puertoPP);
	
	#define TAMANIO_BUFFER 1024
	char buffer[TAMANIO_BUFFER];
	int bytesLeidos = 0;
	char *otroBuffer = NULL;
	
	while((bytesLeidos = read(script, &buffer, TAMANIO_BUFFER)) > 0) {
		otroBuffer = malloc(bytesLeidos + 1);
		memcpy(otroBuffer, buffer, bytesLeidos);
		otroBuffer[bytesLeidos] = '\0';
		printf("Lei la linea: %s\n", otroBuffer);
		free(otroBuffer);
	}
	
	close(script); // TODO: error handling
}
