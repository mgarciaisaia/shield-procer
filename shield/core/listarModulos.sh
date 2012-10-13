#!/bin/bash
if [ -z $1 ]
then
	echo "ERROR - el primer parámetro de $0 debe ser el directorio de configuración de SHIELD"
	exit 1
fi
if [ -z $2 ]
then
	echo "ERROR - el segundo parámetro de $0 debe ser el nombre del archivo de comandos" 
	exit 2
fi
archivoModulos=$1/conf/$2.conf
if [ ! -f $archivoModulos ]
then
	echo "ERROR - $archivoModulos no es un archivo existente"
	exit 3
fi
while read linea
do
	echo ${linea/%:on/}
done <<< `grep ":on$" $archivoModulos | grep -v "$^"`
