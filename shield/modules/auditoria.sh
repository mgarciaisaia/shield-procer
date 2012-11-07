#!/bin/bash

FILE_LOG=$HOME_SHIELD/auditoria.log

# cada vez que un usuario ejecuta un comando se loguea en auditoria.log
function procesar(){
#VERIFICA SI EXISTE EL ARCHIVO LOG LOCAL
	if [ ! -f $FILE_LOG ] ; then
		shieldLog ERROR "${BASH_SOURCE[0]}: no existe el archivo de auditoria local"
		return 1
	fi
	tamanio_actual_log_local=$(du -b $FILE_LOG | awk '{print $1}')
	tamanio_comando=${#1}
	tamanio_archivo_mas_comando=$(($tamanio_actual_log_local + $tamanio_comando))
#COMPRUEBA SI PUEDE LOGUEAR LOCALMENTE O SI LO HACE EN REMOTO
	if (( $tamanio_archivo_mas_comando >= $TAM_MAX_ARCH_LOG )) ; then
		ssh -i $DIRECTORIO_SHIELD/clave_auditoria $HOST_AUDITORIA "echo -e \"$1\" >> auditoria.log"
	else
		echo -e $1 >> ${FILE_LOG}
	fi
}

informacion(){
	tamanio_actual_log_local=$(du -b $FILE_LOG | awk '{print $1}') 
	echo "Dirección del host remoto: $HOST_AUDITORIA"
	echo "Tamaño máximo del archivo de log local: ${TAM_MAX_ARCH_LOG}b"
	echo "Tamaño actual del archivo de log local: ${tamanio_actual_log_local}b"
}

iniciar(){
	. $HOME_SHIELD/conf/auditoria.conf
	retornoCargarConf=$?
	if [ $retornoCargarConf -ne 0 ]
	then
		shieldLog ERROR "${BASH_SOURCE[0]}: error cargando la configuracion del modulo" $retornoCargarConf
		return $retornoCargarConf
	fi
	export TAM_MAX_ARCH_LOG
	export HOST_AUDITORIA
}

detener(){
	unset TAM_MAX_ARCH_LOG
	unset HOST_AUDITORIA
}

case $1 in
	informacion) 
		informacion
		;;
	iniciar)
		iniciar
		;;
	detener)
		detener
		;;
	procesar)
		procesar "$2"
		;;
esac
