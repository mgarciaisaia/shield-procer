#!/bin/bash

function obtenerCantSesionesActuales(){
	USR_SESION=`whoami`;
	CANT_SESIONES=`who | grep -c $USR_SESION`
}

function imprimirInformacion(){
	obtenerCantSesionesActuales
	echo "----------- MODULO CONTROL DE SESIONES -----------"
	echo "- Máxima Cantidad de Sesiones Posibles: $CANT_MAXIMA_SESIONES"
	echo "- Cantidad de Sesiones Activas: $CANT_SESIONES"
	echo "--------------------------------------------------"
}

function iniciar(){
	. $HOME_SHIELD/conf/controlDeSesiones.conf
	export CANT_MAXIMA_SESIONES
	obtenerCantSesionesActuales
	if [ $CANT_SESIONES -gt $CANT_MAXIMA_SESIONES ]; then
		return 1
	fi
}

function detener(){
	unset CANT_MAXIMA_SESIONES
}

case $1 in
	informacion) 
		imprimirInformacion;;
	iniciar)
		iniciar;;
	procesar)
		;; # No hacemos nada
	detener)
		detener;;
esac
