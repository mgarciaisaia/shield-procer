#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	char* direccionPP = getenv("PP_IP");
	char* puertoPP = getenv("PP_Puerto");
	
	printf("Nos conectamos a %s:%s\n", direccionPP, puertoPP);
}
