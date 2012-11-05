#!/bin/bash
while true
do
	for modulo in $@
        do
                $modulo procesar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			# FIXME: loggear
			echo "Comando $modulo dio $codigoSalida"
			kill -6 $PPID # Bajamos la persiana
		fi
        done
	sleep $TIEMPO_EJECUCION_PERIODICOS
done
