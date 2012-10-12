#!/bin/bash

VALOR_RETORNO=0;
function procesar(){
	`ps T -o pcpu,pid,user,nice,tty,args | sort -n -r | grep -v -w shield.sh | grep -v -w /bin/login`
}
case $1 in
	informacion)
	;;
	iniciar)
	;;
	procesar)
		procesar;
	;;
	detener)
	;;
esac

exit $VALOR_RETORNO;
