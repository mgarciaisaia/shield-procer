#!/bin/bash
case $1 in
	informacion) sh ./darInfo.sh;;
	iniciar) . ./iniciar.sh;;
	detener) unset CONFSEGURIDAD;;
	procesar) sh ./buscarComando.sh $2;;
	*) echo "No es una accion que pueda realizar el modulo de seguridad" ;;
esac
