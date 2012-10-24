#!/bin/bash
archivoModulos=$1
if [ -z $archivoModulos ]
then
	echo "ERROR - el primer parámetro de $0 debe ser la ruta al archivo de modulos"
	exit 1
fi
if [ ! -f $archivoModulos ]
then
	echo "ERROR - $archivoModulos no es un archivo existente"
	exit 2
fi
temporalModulos=$(grep --color=no ":on$" $archivoModulos)
while read modulo
do
	echo ${modulo/%:on/}
done <<< "$temporalModulos"
#echo "${temporalModulos//:on$/}"
