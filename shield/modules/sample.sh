#!/bin/bash
if [ $1 = 'iniciar' ]; then
	echo "iniciando..."
	export iniciado='allyourbasearebelongtous'
	return 0
fi
if [ $1 = 'procesar' ]; then
	echo Y yo te digo que $iniciado
	return 0
fi
if [ "$(echo $@ | grep -c a)" -eq 0 ]; then
	exit 0
else
	exit 1
fi
