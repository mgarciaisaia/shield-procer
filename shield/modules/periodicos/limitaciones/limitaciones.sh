#!/bin/bash

source limitaciones.config

#set -x

informacion(){
	echo "MAXIMO USO PERMITIDO DE CPU: $MAX_CPU"
	echo "MAXIMO USO PERMITIDO DE MEMORIA: $MAX_MEMORIA"
	echo "CANTIDAD MAXIMA DE PROCESOS: $MAX_CANTIDAD_PROCESOS"
	echo "MAXIMA CANTIDAD DE SOCKETS: $MAX_CANTIDAD_SOCKETS"
	echo "MAXIMA CANTIDAD DE ARCHIVOS: $MAX_CANTIDAD_ARCHIVOS"

#MAXIMA CANTIDAD DE ARCHIVOS ABIERTOS POR SESION
MAX_CANTIDAD_ARCHIVOS=20
}

iniciar(){
	export MAX_CPU
	export MAX_MEMORIA
	export MAX_CANTIDAD_PROCESOS
	export MAX_CANTIDAD_SOCKETS
	export MAX_CANTIDAD_ARCHIVOS
}

detener(){
	unset MAX_CPU
	unset MAX_MEMORIA
	unset MAX_CANTIDAD_PROCESOS
	unset MAX_CANTIDAD_SOCKETS
	unset MAX_CANTIDAD_ARCHIVOS
}


#FALTA EL MAXIMO DE SOCKETS POR SESION
procesar(){

memoria_total=0
cpu_total=0
procesos_totales=0
archivos_abiertos_totales=0

#EL FLAG LO UTILIZO PARA NO TENER EN CUENTA LA PRIMER LINEA DEL COMANDO 'ps'
#procesos_totales=`ps -o pid,pcpu,pmem |
#	awk 'BEGIN { procesos =  0 ; flag = 0}
#	{ if(flag > 0) procesos++; flag = 1 }
#	END {print procesos}'`

#FORMA ENCONTRADA EN GOOGLE
procesos_totales=$(ps -A r | wc -l)

memoria_total=`ps -o pid,pcpu,pmem | awk ' NR >= 1 ' |
	awk 'BEGIN { memoria = 0 }
	{ memoria+=$3 }
	END {print memoria}'`

cpu_total=`ps -o pid,pcpu,pmem | awk ' NR >= 1 ' |
	awk 'BEGIN { cpu = 0 ; flag = 0}
	{ if(flag > 0) memoria+=$2; flag = 1 }
	END {print cpu}'`

#EN EL FORO UNO LO PROPUSO ASI
sesion=$(tty)
archivos_abiertos_totales=`lsof $sesion | wc -l`

#TIRA ERROR
#=====================================================
#archivos_abiertos_totales=`ps -o pid,pcpu,pmem |
#	awk 'BEGIN { archivos_abiertos = 0; flag = 0 }
#	{ 
#		if (flag > 0){
#		archivos_abiertos+=$(find /proc/$1/fd -type l | wc -l)
#		}
#		flag = 1;
#	}
#'`
#====================================================

#echo "memoria usada: $memoria_total"
#echo "cpu usada: $cpu_total"
#echo "cantidad de procesos: $procesos_totales"
#echo "archivos abiertos por los procesos: $archivos_abiertos_totales"

if (( $MAX_CPU < $cpu_total )) ; then
	echo "error"
#	return error
elif (( $MAX_MEMORIA < $memoria_total )) ; then
	echo "error"
#	return error
elif (( $MAX_CANTIDAD_PROCESOS < $procesos_totales )) ; then
	echo "error"
#	return error
elif (( $MAX_CANTIDAD_ARCHIVOS < $archivos_abiertos_totales )) ; then
	echo "error"
#	return error
else
	echo "TERMINO CORRECTAMENTE"
fi


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
		procesar 
		;;
esac
