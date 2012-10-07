#!/bin/bash
if [ -z $1 ]
then
	echo "ERROR - el primer parámetro de $0 debe ser el directorio de configuración de SHIELD"
	exit 1
fi
while read linea
do
	echo ${linea/%:on/}
done <<< `grep ":on$" $1/conf/comandos.conf | grep -v "$^"`
