#!/bin/bash
if [ -z $1 ]
then
	echo "El primer parámetro debe ser la ruta al directorio core"
	exit -1
fi
DIR_CORE=$1
alias ayuda="$DIR_CORE/mostrarAyuda.sh procesar"
alias apagar="sudo halt -p"
alias listar_modulos='./listar_modulos.sh'
alias salir='exit'
function mostrar { eval echo $`echo $1`; }
