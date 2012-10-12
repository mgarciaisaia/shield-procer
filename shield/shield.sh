#!/bin/bash -i

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)
HOME_SHIELD=~/.shield

function iniciarModulos() {
	modulos=`$DIRECTORIO_SHIELD/core/listarModulos.sh $HOME_SHIELD $1`
	if [ $? -ne 0 ]; then
		echo $modulos # modulos tiene el mensaje de error
		exit $?
	fi
	while read modulo
	do
		. $modulo iniciar
	done <<< "$modulos"
}

iniciarModulos "comandos"
iniciarModulos "periodicos"

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh &

# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT
# Trappeo el cierre de sesion para matar los procesos que quedan vivos
trap ". $DIRECTORIO_SHIELD/utils/limpiarSesion.sh; exit" 0

function ejecutarModulosDeComando() {
	while read linea
	do
		if test -n "$linea" 
		then
			eval "$linea $1"
			retorno=$?
			if [ $retorno -ne 0 ]
			then
				echo "Error ejecutando el modulo $linea - error $retorno"
				return $retorno
			fi
		fi
	done <<< "$modulos"
}

while true
do
	read -e -p "> " comando
	history -s $comando
	if ejecutarModulosDeComando '$comando'
	then
		eval $comando
	fi
done
