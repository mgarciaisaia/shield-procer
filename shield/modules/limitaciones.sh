#!/bin/bash

informacion(){
	leerValoresLimitaciones
	echo "Uso de CPU: ${cpu_total}% (máx: ${MAX_CPU}%)"
	echo "Uso de Memoria: ${memoria_total}KiB (máx: ${MAX_MEMORIA}KiB)"
	echo "Cantidad de Procesos: ${procesos_totales} (máx: ${MAX_CANTIDAD_PROCESOS})"
	echo "Cantidad de Sockets abiertos: ${sockets_totales} (máx: ${MAX_CANTIDAD_SOCKETS})"
	echo "Cantidad de Archivos abiertos: ${archivos_abiertos_totales} (máx: ${MAX_CANTIDAD_ARCHIVOS})"
}

iniciar(){
	. $HOME_SHIELD/conf/limitaciones.conf
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

function sumatoria() {
	total=0
	for numero in "$@"
	do
		total=$(($total + $numero))
	done
	echo $total
}

leerValoresLimitaciones() {
	procesos_totales=$(ps -o pid= | wc -l)

	memoria_total=$(sumatoria $(ps -o rss=))

	cpu_total=`ps -o pid,pcpu,pmem | awk ' NR >= 1 ' |
		awk 'BEGIN { cpu = 0 }
		{ cpu+=$2 }
		END {print cpu}'`

	archivos_abiertos_totales=$(lsof $(tty) | tail -n +2 | wc -l)

	#CALCULA LA CANTIDAD DE SOCKETS UTILIZADOS POR LA SESION
	sockets_totales=0
	for pid in `ps -o pid= `
	do
		if [ -d /proc/$pid/fd ] ; then
			sockets_totales=$((`sudo ls -l /proc/$pid/fd | grep socket: | wc -l` + $sockets_totales))
		fi
	done
}

procesar() {
	leerValoresLimitaciones
	if [ $(awk 'BEGIN{ print '$MAX_CPU'<'$cpu_total' }') -ne 0 ] ; then
		echo "Superado el uso máximo de CPU: ${cpu_total}% (máx: ${MAX_CPU}%)"
		exit 1
	elif (( $MAX_MEMORIA < $memoria_total )) ; then
		echo "Superado el uso máximo de memoria: ${memoria_total}KiB (máx: ${MAX_MEMORIA}KiB)"
		exit 1
	elif (( $MAX_CANTIDAD_PROCESOS < $procesos_totales )) ; then
		echo "Superada la cantidad máxima de procesos: ${procesos_totales} (máx: ${MAX_CANTIDAD_PROCESOS})"
		exit 1
	elif (( $MAX_CANTIDAD_ARCHIVOS < $archivos_abiertos_totales )) ; then
		echo "Superada la cantidad máxima de archivos abiertos: ${archivos_abiertos_totales} (máx: ${MAX_CANTIDAD_ARCHIVOS})"
		exit 1
	elif (( $MAX_CANTIDAD_SOCKETS < $sockets_totales )) ; then
		echo "Superada la cantidad máxima de sockets abiertos: ${sockets_totales} (máx: ${MAX_CANTIDAD_SOCKETS})"
		exit 1
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
