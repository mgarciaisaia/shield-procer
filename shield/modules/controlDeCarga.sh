#!/bin/bash
NOMBRE_MODULO=${BASH_SOURCE[0]}

function calcularValoresDeCarga() {
	procesoMayor=$(ps -o pcpu= -o pid= --sort -pcpu | head -n1)
	cpuProcesoMayor=$(echo $procesoMayor | cut -d' ' -f1)
	if [ $(awk 'BEGIN{ print '$CPU_MAXIMA_PROCESO'<'$cpuProcesoMayor' }') -ne 0 ]
	then
		pidProcesoMayor=$(echo $procesoMayor | cut -d' ' -f2)
	else
		pidProcesoMayor=0
	fi
}

function procesar() {
	calcularValoresDeCarga
	if [ $pidProcesoMayor -eq 0 ] # Ningun proceso supera
	then
		return 0
	fi

	shieldLog INFO "$NOMBRE_MODULO: el proceso $pidProcesoMayor está usando un ${cpuProcesoMayor}% de CPU, mayor al límite de ${CPU_MAXIMA_PROCESO}%"
	if [ $pidProcesoMayor = $pidUltimoNice ]
	then
		if [ $cantidadNiceConsecutivos -ge 3 ]
		then
			shieldLog INFO "$NOMBRE_MODULO: Elimino el proceso $pidProcesoMayor por tener el 4to nice consecutivo"
			kill -9 $pidProcesoMayor
			return
		fi
	else
		pidUltimoNice=$pidProcesoMayor
		cantidadNiceConsecutivos=0
	fi
	renice +5 $pidProcesoMayor > /dev/null
	cantidadNiceConsecutivos=$(($cantidadNiceConsecutivos + 1))
	shieldLog INFO "$NOMBRE_MODULO: $pidProcesoMayor tiene $cantidadNiceConsecutivos nice consecutivos"
}

function informacion() {
	calcularValoresDeCarga
	echo "Máximo uso de CPU permitido: ${CPU_MAXIMA_PROCESO}%"
	if [ $pidProcesoMayor -ne 0 ]
	then
		echo "Proceso con mayor consumo: $pidProcesoMayor (${cpuProcesoMayor}%)"
		echo "Nice del proceso ${pidProcesoMayor}: $(ps -p $pidProcesoMayor -o ni=)"
		if [ $pidProcesoMayor = $pidUltimoNice ]
		then
			echo "Cantidad de incrementos de nice consecutivos de $pidProcesoMayor: $cantidadNiceConsecutivos"
		else
			echo "El proceso $pidProcesoMayor no tiene incrementos de nice consecutivos"
			if [ $pidUltimoNice -ne 0 ]
			then
				echo
				echo "El proceso $pidUltimoNice tiene $cantidadNiceConsecutivos seguidos, con un nice actual de $(ps -p $pidUltimoNice -o ni=)"
			fi
		fi
	else
		echo Ningún proceso está superando el máximo de consumo de CPU permitido
	fi
}

case $1 in
	informacion)
		informacion
	;;
	iniciar)
		. $HOME_SHIELD/conf/controlDeCarga.conf
		export CPU_MAXIMA_PROCESO
		export pidUltimoNice=0
		export cantidadNiceConsecutivos=0
	;;
	procesar)
		procesar;
	;;
	detener)
		unset CPU_MAXIMA_PROCESO
		unset pidUltimoNice
		unset cantidadNiceConsecutivos
	;;
esac
