#!/bin/bash 

set -x
#LEVANTAR POR ARCHIVO DE CONFIGURACION
CONFSEGURIDAD="ls,mkdir"

#CAMBIA EL DELIMITADOR
IFS=","
for comando_prohibido in $CONFSEGURIDAD;
do
	RESULTADO=`echo $1 | grep -w $comando_prohibido`
	if [ ! -z $RESULTADO  ] ; then
		echo "ERROR"
	fi
done 

