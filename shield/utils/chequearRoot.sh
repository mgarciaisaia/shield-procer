#!/bin/bash
## TODO: agregar un parametro -s que evite los echo (para usar desde el make)

## FIXME: meter inline el idUsuario
idUsuario=$(id -u)
if (( $idUsuario )); then
	echo "No sos root :("
	exit -1
else
	echo "Root para todos!! :D";
fi
