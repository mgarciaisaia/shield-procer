#!/bin/bash

source auditoria.config

# Tengo que tener el ip host para hacer el logueo remoto
FILE_LOG=${RUTA_ARCHIVO_LOG}${USER}.txt

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
	archivo=${USER}.txt
	ssh localhost "echo -e $1 >> $archivo"
#	ssh $IP:"echo $1 >> $ubicacion_archivo"
}

informacion(){
	tamanio_actual_log_local=$(du -b $FILE_LOG | awk '{print $1}') 
	echo "Tamaño actual del archivo log local: $tamanio_actual_log_local"
	echo "Dirección del host remoto: $IP"
}

case $1 in
	informacion) 
		informacion
		;;
	iniciar)
#HAY QUE "EXPORTAR" LAS VARIABLES, NO UTILIZAR EL SOURCE QUE HAGO AL PRINCIPIO
		echo "iniciar"
		;;
	detener)
#HACER UNSET DE LAS VARIABLES EXPORTADAS
		echo "detener"
		;;
	procesar)
		procesar $2
		;;
esac
