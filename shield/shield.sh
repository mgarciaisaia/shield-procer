#!/bin/bash

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)




$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh &
while true
do
	read -e comando
	echo "Ingresaste $comando"
	eval $comando
done
