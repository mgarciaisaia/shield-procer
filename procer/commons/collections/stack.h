/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STACK_H_
#define STACK_H_

	#include "list.h"

	typedef struct {
		t_list* elements;
	} t_stack;

	t_stack *stack_create();
	void stack_destroy(t_stack *);
	void stack_destroy_and_destroy_elements(t_stack*, void(*element_destroyer)(void*));

	void stack_push(t_stack *, void *element);
	void *stack_pop(t_stack *);
	void *stack_peek(t_stack *);
	void stack_clean(t_stack *);
	void stack_clean_and_destroy_elements(t_stack *, void(*element_destroyer)(void*));

	int stack_size(t_stack *);
	int stack_is_empty(t_stack *);

	void stack_iterate(t_stack* self, void(*closure)(void*));

#endif /*STACK_H_*/
