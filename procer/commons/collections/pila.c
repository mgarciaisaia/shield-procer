/*
 * pila.c
 *
 *  Created on: 07/11/2012
 *      Author: utnso
 */

/*
 Agrega un nodo al inicio de la lista ligada
 *pila es el apuntador que apunta al primer nodo de la lista ligada (la cima de la pila)
 */

#include "pila.h"
#include <stdlib.h>
#include <stdio.h>

void pila_push(ptrPila *pila, t_registro_stack registro) {
	// Crea un nuevo nodo
	ptrNodo nodo;
//	nodo->registro->retorno = malloc(sizeof(uint32_t));
//	nodo->registro->nombre_funcion = calloc(1,5);
	nodo = (ptrNodo) malloc(sizeof(nodo_t));
	if (nodo != NULL) {
		nodo->registro = registro;
		// El apuntador nodo->siguiente va a apuntar al primer nodo de la lista ligada
		nodo->siguiente = *pila;
		// pila va a apuntar al nuevo nodo, con esto hacemos que el nuevo nodo sea ahora el primer nodo de la lista ligada
		*pila = nodo;
	}
}

/*
 Elimina el primer nodo de la lista ligada
 *pila es el apuntador que apunta al primer nodo de la lista ligada (la cima de la pila)
 */
t_registro_stack pila_pop(ptrPila *pila) {
	// Crea un nuevo nodo
	ptrNodo nodo;
	t_registro_stack registro;

	// El nuevo nodo va a apuntar al primer nodo de la lista ligada
	nodo = *pila;
	registro = (*pila)->registro;
	// Ahora el segundo nodo de la lista ligada va a ser el primero
	*pila = (*pila)->siguiente;
	// Borra el primer nodo de la lista ligada
//	free(nodo->registro->retorno);
	free(nodo);
	// Regresa el valor que contenía el nodo que se eliminó
	return registro;
}

/*
 Regresa 1 si no hay nodos en la lista ligada y cero en caso contrario
 *pila es el apuntador que apunta al primer nodo de la lista ligada (la cima de la pila)
 */
int pila_vacia(ptrPila *pila) {
	return (*pila == NULL ? 1 : 0);
}

/*
 Muestra los datos de los nodos
 */
void nodos_pila(ptrNodo nodo) {
	if (nodo == NULL)
		printf("La pila está vacia\n");
	else {
		while (nodo != NULL) {
			//todo: ver porq rompe!!!!!
//    	  	 printf("%d\n",nodo.registro.retorno);
//           printf("%s\n",nodo.registro.nombre_funcion);
			nodo = nodo->siguiente;
		}
		printf("\n");
	}
}
