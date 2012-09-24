#!/bin/bash
case $1 in
	informacion) sh /home/utnso/tpos/2012-2c-no-quiero-matarte-pero-si-me-obligas/shield/modules/darInfo.sh;;
	iniciar) . /home/utnso/tpos/2012-2c-no-quiero-matarte-pero-si-me-obligas/shield/modules/iniciar.sh;;
	detener) unset CONFSEGURIDAD;;
	procesar) sh /home/utnso/tpos/2012-2c-no-quiero-matarte-pero-si-me-obligas/shield/modules/buscarComando.sh $2;;
	*) echo "No es una accion que pueda realizar el modulo de seguridad" ;;
esac
