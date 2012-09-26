#!/bin/bash

TAM_MAX_ARCH_LOG=10
HOSTNAME=/home/utnso

# cada vez que un usuario ejecuta un comando se loguea en auditoria.txt
function procesar {
	if [ !supera_tamanio_log $2  ] ; then
		echo $2 >> auditoria.txt
	else
		log_remoto $2
	fi
}

function supera_tamanio_log {
	tamanio_comando=$(expr length $1)
	return $(( $tamanio_comando >= $TAM_MAX_ARCH_LOG ))
}

function log_remoto {
	ubicacion_archivo=auditoria.txt
	ssh $HOSTNAME "echo $2 >> $ubicacion_archivo"
}
