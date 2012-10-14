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

function inicializarModulos() {
	while read modulo
	do
		. $modulo iniciar
	done <<< "$1"
}

function iniciarRegistrarModulos() {
	for pid in `jobs -p`
	do
		kill -9 $pid 2> /dev/null
	done
	archivoComandos=$HOME_SHIELD/conf/comandos.conf
	archivoPeriodicos=$HOME_SHIELD/conf/periodicos.conf
	hashComandos=$(md5sum $archivoComandos)
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
	hashPeriodicos=$(md5sum $archivoPeriodicos)
	inicializarModulos $modulosComando
	inicializarModulos $modulosPeriodicos
}

iniciarRegistrarModulos

. $DIRECTORIO_SHIELD/core/cargarBuiltins.sh $DIRECTORIO_SHIELD/core

$DIRECTORIO_SHIELD/utils/verificarPeriodicos.sh $modulosPeriodicos &

# Ignoramos la SIGINT (CTRL + C)
trap "" SIGINT
# Trappeo el cierre de sesion para matar los procesos que quedan vivos
trap ". $DIRECTORIO_SHIELD/utils/limpiarSesion.sh; exit" 0

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
