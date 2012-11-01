#!/bin/bash -i

SCRIPT_SHIELD=$(readlink -f $0)
DIRECTORIO_SHIELD=$(dirname $SCRIPT_SHIELD)
HOME_SHIELD=~/.shield
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
		kill $pidPeriodicos
	fi
	for moduloPeriodico in $modulosPeriodicos
	do
		$moduloPeriodico detener
	done
	for moduloComando in $modulosComando
	do
		$moduloComando detener
	done
}

function inicializarModulos() {
	for moduloPeriodico in $modulosPeriodicos
	do
		$moduloPeriodico iniciar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			# FIXME: loggear
			echo "$codigoSalida - BUAAAA!"
			exit $?
		fi
	done
	for moduloComando in $modulosComando
	do
		$moduloComando iniciar
		codigoSalida=$?
		if [ $codigoSalida -ne 0 ]
		then
			# FIXME: loggear
			echo "$codigoSalida - BUAAAA!"
			exit $?
		fi
	done
}

function iniciarRegistrarModulos() {
	detenerModulos
	archivoComandos=$HOME_SHIELD/conf/comandos.conf
	archivoPeriodicos=$HOME_SHIELD/conf/periodicos.conf
	modulosComando=`$DIRECTORIO_SHIELD/core/listarModulos.sh $archivoComandos`
	if [ $? -ne 0 ]; then
		echo $modulosComando # modulos tiene el mensaje de error
		exit $?
	fi
	modulosPeriodicos=`$DIRECTORIO_SHIELD/core/listarModulos.sh $archivoPeriodicos`
	if [ $? -ne 0 ]; then
		echo $modulosPeriodicos # modulos tiene el mensaje de error
		exit $?
	fi
	inicializarModulos
	
	$DIRECTORIO_SHIELD/utils/ejecutarPeriodicos.sh "$modulosPeriodicos" &
	pidPeriodicos=$! # consigue el PID del ultimo proceso que tire en background
	
	$DIRECTORIO_SHIELD/utils/verificarConfiguracion.sh $archivoComandos $archivoPeriodicos &
}

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

iniciarRegistrarModulos


# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT

# Cuando llega SIGUSR2, recargamos los modulos
trap iniciarRegistrarModulos SIGUSR2

# Trappeo el cierre de sesion para matar los procesos que quedan vivos
trap detenerModulos 0

function ejecutarModulosDeComando() {
	while read linea
	do
		if test -n "$linea" 
		then
			eval "$linea procesar $1"
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
	read -e -p "$USER - SHIELD: `pwd`> " comando
	history -s $comando
	if ejecutarModulosDeComando '$comando'
	then
		eval $comando
	fi
done
