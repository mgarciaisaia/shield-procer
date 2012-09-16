#!/bin/bash -i

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)
HOME_SHIELD=~/.shield

modulos=`$DIRECTORIO_SHIELD/core/cargarModulos.sh $HOME_SHIELD`

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh &

# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT
# Trappeo el cierre de sesion para matar los procesos que quedan vivos
trap ". $DIRECTORIO_SHIELD/utils/limpiarSesion.sh; exit" 0

function ejecutarModulosDeComando() {
	while read linea
	do
		eval "$linea $1"
		retorno=$?
		if [ $retorno -ne 0 ]
		then
			echo "Error ejecutando el modulo $linea - error $retorno"
			return $retorno
		fi
#		echo "Linea: $linea"
	done <<< "$modulos"
#	return `echo $* | grep a | wc -l`
}



while true
do
	read -e -p "> " comando
	if ejecutarModulosDeComando $comando
	then
		eval $comando
	fi
done
