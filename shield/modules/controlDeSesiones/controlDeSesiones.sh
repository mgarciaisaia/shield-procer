#!/bin/bash

#Comentario de prueba

VALOR_RETORNO=1;
case $1 in
	informacion) 
		echo información;;
	iniciar)
		echo iniciar
		USR_SESION=`whoami`;
		echo $USR_SESION;
		echo 2
		LISTA=`who | grep  $USR_SESION`
		echo 1
		echo $LISTA
		;;
	procesar)
		echo procesar;;
esac

exit $VALOR_RETORNO;
