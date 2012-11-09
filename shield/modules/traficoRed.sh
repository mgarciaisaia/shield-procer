#!/bin/bash

#
# Módulo de limitación de tráfico de red
#
# Controla el tráfico de red poniendo un máximo a la cantidad de paquetes
# enviados por todas las interfaces no-loopback entre cada chequeo
#
# En caso de detectar que se supera ese límite, elimina todos los procesos
# de la sesión que tengan conexiones exernas
#

function contabilizarPaquetesSalientes() {
	totalPaquetesActuales=0
	while read adaptador
	do
		totalPaquetesActuales=$(($totalPaquetesActuales + $(echo $adaptador | cut -d' ' -f11)))
	done <<< "$(cat /proc/net/dev | tail -n +3 | grep -vw lo:)"
	paquetesSalientes=$(($totalPaquetesActuales - $PAQUETES_SALIENTES_ACTUALES))
}

function iniciar() {
	. $HOME_SHIELD/conf/traficoRed.conf
	export MAXIMO_PAQUETES_SALIENTES
	PAQUETES_SALIENTES_ACTUALES=0
	contabilizarPaquetesSalientes
	export PAQUETES_SALIENTES_ACTUALES=$paquetesSalientes
}

function detener() {
	unset MAXIMO_PAQUETES_SALIENTES
	unset PAQUETES_SALIENTES_ACTUALES
}

function informacion() {
	contabilizarPaquetesSalientes
	echo "Cantidad de nuevos paquetes salientes: $paquetesSalientes (máx: $MAXIMO_PAQUETES_SALIENTES)"
}

function procesar() {
	contabilizarPaquetesSalientes
	if [ $paquetesSalientes -gt $MAXIMO_PAQUETES_SALIENTES ]
	then
		ipsLocales=$(ifconfig | grep 'inet:' | cut -d: -f2 | cut -d' ' -f1 | paste -sd\|)
		procesosDeLaSesion=$(ps -o pid= | paste -sd\|)
		
		while read conexion
		do
			ipDestino=$(echo $conexion | cut -d' ' -f5 | cut -d':' -f1)
			socketProcesoConexion=$(echo $conexion | cut -d' ' -f4)

			# El ejército de barritas y palitos en ${ipsLocales//\|/\\\|} reemplaza
			# los | por \| para que sea un OR en grep

			if [ $(echo $ipDestino | grep -cv ${ipsLocales//\|/\\\|}) -ne 0 ] # si la linea NO tiene las IPs locales
			then
				pidProcesoConexion=$(echo $conexion | cut -d' ' -f7 | cut -d '/' -f1)
				if [ $(echo $pidProcesoConexion | grep -c ${procesosDeLaSesion//\|/\\\|}) -ne 0 ]
				then
					shieldLog INFO "${BASH_SOURCE[0]}: Elimino el proceso $pidProcesoConexion que tenía abierto el socket $socketProcesoConexion"
					kill -9 $pidProcesoConexion 2>/dev/null # Procesos con N sockets darían N-1 errores
				fi
			fi
		done <<< "$(netstat -ntp 2>/dev/null | grep '^tcp' | tr -s ' ')" # tr -s simplifica caracteres repetidos
	fi
	PAQUETES_SALIENTES_ACTUALES=$(($PAQUETES_SALIENTES_ACTUALES + $paquetesSalientes))
}

case $1 in
	iniciar)
		iniciar;;
	informacion)
		informacion;;
	procesar)
		procesar;;
	detener)
		detener;;
esac
