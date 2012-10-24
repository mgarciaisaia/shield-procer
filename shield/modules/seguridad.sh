#!/bin/bash

case $1 in
	informacion) ./darInfo.sh;;
	iniciar) . /iniciar.sh;;
	detener) unset CONFSEGURIDAD;;
	procesar) ./buscarComando.sh $2;;
	*) echo "No es una accion que pueda realizar el modulo de seguridad" ;;
esac
