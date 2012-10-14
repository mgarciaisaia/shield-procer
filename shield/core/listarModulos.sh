#!/bin/bash
if [ -z $1 ]
then
	echo "ERROR - el primer parámetro de $0 debe ser la ruta al archivo de modulos"
	exit 1
fi
archivoModulos=$1
if [ ! -f $archivoModulos ]
then
	echo "ERROR - $archivoModulos no es un archivo existente"
	exit 2
fi
while read linea
do
	echo ${linea/%:on/}
done <<< `grep ":on$" $archivoModulos | grep -v "$^"`
