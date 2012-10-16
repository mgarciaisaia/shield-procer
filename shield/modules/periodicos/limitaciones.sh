#!/bin/bash

procesar(){
	if supera_max_uso_cpu ; then
#		return error
	elif supera_max_uso_memoria ; then
		return error
	elif supera_max_procesos ; then
#		return error
	elif supera_max_sockets ; then
#		return error
	elif supera_max_arch_abiertos ; then
#		return error
	fi
}

supera_max_uso_cpu(){

}

case $1 in
	informacion) 
		informacion
		;;
	iniciar)
		iniciar
		;;
	detener)
		detener
		;;
	procesar)
		procesar $2
		;;
esac
