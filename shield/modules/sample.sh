#!/bin/bash
if [ $1 = 'iniciar' ]; then
	echo "iniciando..."
	export iniciado='allyourbasearebelongtous'
	return 0
fi
if [ $1 = 'procesar' ]; then
	echo Y yo te digo que $iniciado
	return
fi
if [ $1 = 'informacion' ]; then
	echo "TUNGA!"
fi
if [ "$(echo $@ | grep -c a)" -eq 0 ]; then
	return 0
else
	return 1
fi
