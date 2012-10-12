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
	echo "- Máxima Cantidad de Sesiones Posibles: " $CANT_MAXIMA_SESIONES
	echo "- Cantidad de Sesiones Activas: " $CANT_SESIONES
	echo "--------------------------------------------------"
}

VALOR_RETORNO=0;

case $1 in
	informacion) 
		echo información;
		imprimirInformacion;;
	iniciar)
		echo iniciar;
		obtenerCantSesionesActuales;
		if [ $CANT_SESIONES -gt $CANT_MAXIMA_SESIONES ]; then
			echo "Más sesiones de las permitidas"
			VALOR_RETORNO=1;
		else
			echo "Menos sesiones de las permitidas";
			VALOR_RETORNO=0;
		fi
		;;
	procesar)
		echo procesar;;
esac

exit $VALOR_RETORNO;
