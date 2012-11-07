#!/bin/bash
archivoModulos=$1
if [ -z $archivoModulos ]
then
	shieldLog ERROR "${BASH_SOURCE[0]}: el primer parámetro debe ser la ruta al archivo de modulos"
	exit 1
fi
if [ ! -f $archivoModulos ]
then
	shieldLog ERROR "${BASH_SOURCE[0]}: $archivoModulos no es un archivo existente"
	exit 2
fi
temporalModulos=$(grep --color=no ":on$" $archivoModulos)
while read modulo
do
	echo ${modulo/%:on/}
done <<< "$temporalModulos"
