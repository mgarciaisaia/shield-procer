#!/bin/bash -i

export SCRIPT_SHIELD=$(readlink -f $0)
export DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)
export HOME_SHIELD=~/.shield

export TIEMPO_MONITOR_CONFIGURACION=5
export TIEMPO_EJECUCION_PERIODICOS=10

# overridea variables desde archivo de configuracion
if [ -f $HOME_SHIELD/conf/shield.conf ]
then
	. $HOME_SHIELD/conf/shield.conf
fi

function shieldLog() {
	nivelLog=$1
	mensajeLog=$2
	#Opcional
	if [ -n "$3" ]
	then
		codigoError=" - Exit code: $3"
	else
		codigoError=''
	fi


	mensaje="$nivelLog - $mensajeLog$codigoError"
	echo "$mensaje"
	echo "$mensaje" >> $HOME_SHIELD/shell.log
}

export -f shieldLog

echo " ======== BIENVENIDO A SHIELD ======== "
echo "
           |\`-._/\_.-\`|
           |    ||    |
           |___o()o___|
           |__((<>))__|
           \   o\/o   /
            \   ||   /
             \  ||  /
              '.||.'
                ``"


function detenerModulos() {
	if [ -v pidPeriodicos ]
	then
		kill $pidPeriodicos 2> /dev/null
	fi
	if [ -v pidVerificarConfiguracion ]
	then
		kill $pidVerificarConfiguracion 2> /dev/null
	fi
	for moduloPeriodico in $modulosPeriodicos
	do
		. $moduloPeriodico detener
	done
	for moduloComando in $modulosComando
	do
		. $moduloComando detener
	done
}

function inicializarModulos() {
	for moduloPeriodico in $modulosPeriodicos
	do
		. $moduloPeriodico iniciar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			shieldLog ERROR "Error inicializando el modulo $moduloPeriodico" $codigoSalida
			exit $codigoSalida
		fi
	done
	for moduloComando in $modulosComando
	do
		. $moduloComando iniciar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			shieldLog ERROR "Error inicializando el modulo $moduloComando" $codigoSalida
			exit $codigoSalida
		fi
	done
}

function iniciarRegistrarModulos() {
	detenerModulos
	archivoComandos=$HOME_SHIELD/conf/comandos.conf
	archivoPeriodicos=$HOME_SHIELD/conf/periodicos.conf
	modulosComando=`$DIRECTORIO_SHIELD/core/listarModulos.sh $archivoComandos`
	codigoSalida=$?
	if [ $codigoSalida -ne 0 ]; then
		shieldLog ERROR "$modulosComando" $codigoSalida # modulosComando tiene el mensaje de error
		exit $codigoSalida
	fi
	modulosPeriodicos=`$DIRECTORIO_SHIELD/core/listarModulos.sh $archivoPeriodicos`
	codigoSalida=$?
	if [ $codigoSalida -ne 0 ]; then
		shieldLog ERROR "$modulosPeriodicos" $codigoSalida # modulosPeriodicos tiene el mensaje de error
		exit $codigoSalida
	fi
	inicializarModulos
	
	$DIRECTORIO_SHIELD/utils/ejecutarPeriodicos.sh "$modulosPeriodicos" &
	pidPeriodicos=$! # consigue el PID del ultimo proceso que tire en background
	
	$DIRECTORIO_SHIELD/utils/verificarConfiguracion.sh $archivoComandos $archivoPeriodicos &
	pidVerificarConfiguracion=$!
}

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

iniciarRegistrarModulos


# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT

# Cuando llega SIGUSR2, recargamos los modulos
trap iniciarRegistrarModulos SIGUSR2

# Trappeo el cierre de sesion para matar los procesos que quedan vivos
trap "detenerModulos && pkill -P $$" 0

function ejecutarModulosDeComando() {
	for moduloComando in $modulosComando
	do
		. $moduloComando procesar "$1"
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			shieldLog ERROR "Error ejecutando el modulo $moduloComando" $codigoSalida
			return $codigoSalida
		fi
	done
}

while true
do
	read -e -p "$USER - SHIELD: `pwd`> " comando
	history -s $comando
	if ejecutarModulosDeComando "$comando"
	then
		eval $comando
	fi
done
