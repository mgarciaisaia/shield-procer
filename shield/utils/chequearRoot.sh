#!/bin/bash
verbose=1
if [[ ( "--quiet" == $1 ) || ( "-s" == $1 ) ]] # ) -o ( $1 = "-s" ) ]]
then
	verbose=0
fi

if [ "$(id -u)" -ne 0 ]; then
	mensaje="No sos root :("
	valor=1
else
	mensaje="Root para todos!! :D"
	valor=0
fi

if [ $verbose -ne 0 ]; then
	echo $mensaje
fi
exit $valor
