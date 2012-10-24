#!/bin/bash
hashInicial=`cat $@ | md5sum`
hashConfiguracion=`cat $@ | md5sum`

while [ "$hashConfiguracion" = "$hashInicial" ]
do
	sleep 5
	hashConfiguracion=`cat $@ | md5sum`
done
kill -s SIGUSR2 $PPID
