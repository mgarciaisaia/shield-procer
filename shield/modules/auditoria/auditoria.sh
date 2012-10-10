#!/bin/bash

# Tengo que tener el ip host para hacer el logueo remoto
UBICACION_LOCAL_LOG=/home/utnso/2012-2c-no-quiero-matarte-pero-si-me-obligas/shield/modules/auditoria/
TAM_MAX_ARCH_LOG=1000
FILE_LOG=${UBICACION_LOCAL_LOG}${USER}_log_comandos.txt

set -x

# cada vez que un usuario ejecuta un comando se loguea en auditoria.txt
function procesar(){

#VERIFICA SI EXISTE EL ARCHIVO LOG LOCAL
	if [ -e $FILE_LOG ] ; then 
		tamanio_actual_log_local=$(du -b $FILE_LOG | awk '{print $1}') 
	else 
		tamanio_actual_log_local=0
	fi
	tamanio_comando=$(expr length $1)
	tamanio_archivo_mas_comando=$(($tamanio_actual_log_local + $tamanio_comando))
#COMPRUEBA SI PUEDE LOGUEAR LOCALMENTE O SI LO HACE EN REMOTO
	if (( $tamanio_archivo_mas_comando >= $TAM_MAX_ARCH_LOG )) ; then
		log_remoto $1
	else
		echo -e $1 >> ${FILE_LOG}
	fi
}

# GENERAR LA CLAVE PUBLICA PARA PROBAR EL LOG REMOTO
function log_remoto(){
	archivo=${USER}_log_comandos.txt
	ssh localhost "echo -e $1 >> $archivo"
#	ssh $ip:"echo $1 >> $ubicacion_archivo"
}

echo $#
if (( $# < 2 )) ; then
	echo "necesita 2 parametros"
else
	procesar $1 $2
#	codigo_de_salida_exitoso
fi

