#!/bin/bash
while true
do
	for modulo in $@
        do
                . $modulo procesar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			shieldLog ERROR "Falló el módulo periódico $modulo" $codigoSalida
			kill -6 $PPID # Bajamos la persiana
		fi
        done
	sleep $TIEMPO_EJECUCION_PERIODICOS
done
