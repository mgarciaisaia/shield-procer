/*
 * configuracion.h
 *
 *  Created on: 29/11/2012
 *      Author: utnso
 */

#include "commons/config.h"
#include <stdint.h>
#include <semaphore.h>

#define PATH_CONFIG "/home/utnso/Desarrollo/2012-2c-no-quiero-matarte-pero-si-me-obligas/procer/pp/archivo_configuracion.conf"
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 1000 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

void inicializar_configuracion(void);
void asignar_parametros_que_cambian_en_tiempo_de_ejecucion(t_config *);
void asignar_algoritmo_de_ordenamiento(t_config *);
void asignar_lista_lap(t_config *);

extern t_config * config;
extern uint32_t puerto_tcp;
extern uint8_t time_sleep;
extern uint8_t time_io;
extern sem_t * threads_iot;

