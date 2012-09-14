#!/bin/bash

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)


$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh &
while true
do
	#TODO: evitar que se cierre con CTRL+C ?
	read -e -p "> " comando
	eval $comando
done
