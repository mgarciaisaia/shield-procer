#!/bin/bash -i

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh &

# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT

while true
do
	#TODO: evitar que se cierre con CTRL+C ?
	read -e -p "> " comando
	eval $comando
done
