#!/bin/bash
if [ -z $1 ]
then
	echo "El primer parámetro debe ser la ruta al directorio core"
	exit -1
fi
DIR_CORE=$1
alias ayuda="$DIR_CORE/mostrarAyuda.sh"
alias apagar="sudo halt -p"
alias salir='exit'
function mostrar { eval echo $`echo $1`; }
function listar_modulos {
	echo "== Modulos de comando ==
$modulosComando

== Modulos periodicos ==
$modulosPeriodicos"
}
alias actualizar_modulos="iniciarRegistrarModulos"
function info_modulos {
	for modulo in `echo $modulosComando | grep "$1" `
	do
		$modulo informacion
	done
	for modulo in `echo $modulosPeriodicos | grep "$1" `
	do
		$modulo informacion
	done
}
