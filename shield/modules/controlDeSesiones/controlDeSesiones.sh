#!/bin/bash
source controlDeSesiones.conf
#Comentario de prueba
echo $CANT_MAXIMA_SESIONES

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
	export CANT_MAXIMA_SESIONES;
	obtenerCantSesionesActuales;
	if [ $CANT_SESIONES -gt $CANT_MAXIMA_SESIONES ]; then
		VALOR_RETORNO=1;
	else
		VALOR_RETORNO=0;
	fi
}

function detener(){
	unset CANT_MAXIMA_SESIONES;
}

VALOR_RETORNO=0;

case $1 in
	informacion) 
		echo información;
		imprimirInformacion;;
	iniciar)
		echo iniciar;
		iniciar;
		;;
	procesar)
		echo procesar;;
	detener)
		echo detener;
		detener;
		;;
esac

exit $VALOR_RETORNO;
