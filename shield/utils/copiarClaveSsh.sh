#!/bin/bash

if [ -z $1 ]
then
	echo "Error de parametros - no se recibio la ruta al script de SHIELD"
	echo "USO: $0 /enlace/a/shield.sh usuario"
	exit 1
fi
if [ -z $2 ]
then
	echo "Error de parametros - no se recibio el usuario a configurar" 
	echo "USO: $0 /enlace/a/shield.sh usuario"
	exit 2
fi

ARCHIVO_CONF=$(dirname "$0")/../configuraciones/auditoria.conf

. $ARCHIVO_CONF
if [ -z $HOST_AUDITORIA ]
then
	echo "Error de configuracion - el archivo $ARCHIVO_CONF debe definir la variable HOST_AUDITORIA" 
	echo "Ejemplo:"
	echo "HOST_AUDITORIA=192.168.4.24"
	exit 3
fi

SHIELD_DIR=$(dirname $(readlink "$1"))

echo "*** Copiando clave de SSH al host $HOST_AUDITORIA"
echo
echo "Loggueate al servidor si SSH te lo pide"
echo
ssh-copy-id -i "$SHIELD_DIR"/clave_auditoria "$2"@"$HOST_AUDITORIA" > /dev/null

codigoSalida=$?
if [ $codigoSalida -eq 0 ]
then
	echo "Clave copiada con éxito"
else
	echo "Error copiando clave"
fi
exit $codigoSalida
