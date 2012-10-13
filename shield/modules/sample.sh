#!/bin/bash
if [ $1 = 'iniciar' ]; then
	echo "iniciando..."
	export iniciado='allyourbasearebelongtous'
	return 0
fi
echo $iniciado
if [ "$(echo $@ | grep -c a)" -eq 0 ]; then
	exit 0
else
	exit 1
fi
