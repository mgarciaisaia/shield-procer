/*
 * pila.h
 *
 *  Created on: 07/11/2012
 *      Author: utnso
 */

#ifndef PILA_H_
#define PILA_H_

	#include <stdint.h>

	typedef struct {
		uint32_t retorno;
		char * nombre_funcion;
	} t_registro_stack;

	// Estructura de nodos para una pila
	typedef struct nodo_s {
		t_registro_stack registro;
		struct nodo_s *siguiente;
	} nodo_t;

	typedef nodo_t *ptrNodo;
	typedef nodo_t *ptrPila;

	void pila_push(ptrPila *, t_registro_stack);
	t_registro_stack pila_pop(ptrPila *);
	int pila_vacia(ptrPila *);
	void nodos_pila(ptrNodo);
        
        void pila_hacer(ptrPila pila, void (*bloque)(t_registro_stack *));


#endif /* PILA_H_ */
