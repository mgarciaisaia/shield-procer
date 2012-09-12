#!/bin/bash
## TODO: agregar un parametro -s que evite los echo (para usar desde el make)

if [ "$(id -u)" -ne 0 ]; then
	echo "No sos root :("
	exit -1
else
	echo "Root para todos!! :D";
fi
