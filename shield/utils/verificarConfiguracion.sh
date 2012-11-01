#!/bin/bash
hashInicial=`cat $@ | md5sum`
hashConfiguracion=`cat $@ | md5sum`

while [ "$hashConfiguracion" = "$hashInicial" ]
do
	sleep $TIEMPO_MONITOR_CONFIGURACION
	hashConfiguracion=`cat $@ | md5sum`
done
kill -s SIGUSR2 $PPID
