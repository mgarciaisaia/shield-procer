#!/bin/bash
function leerConfiguracionSeguridad() {
	while read comandoProhibido
	do
		echo $comandoProhibido
	done <<< "$(grep -v '^#' $HOME_SHIELD/conf/seguridad.conf)"
}

function iniciarSeguridad() {
	export COMANDOS_PROHIBIDOS=$(leerConfiguracionSeguridad)
}

function mostrarComandosProhibidos() {
	echo Comandos prohibidos:
	echo "$COMANDOS_PROHIBIDOS"
}

function filtrarComandosProhibidos() {
	for comando_prohibido in $COMANDOS_PROHIBIDOS
	do
		RESULTADO=`echo "$1" | grep -wc $comando_prohibido`
		if [ $RESULTADO -ne 0  ] ; then
			return 1
		fi
	done 
}

case $1 in
	informacion)
		mostrarComandosProhibidos;;
	iniciar)
		iniciarSeguridad;;
	detener)
		unset COMANDOS_PROHIBIDOS;;
	procesar)
		filtrarComandosProhibidos "$2";;
esac
