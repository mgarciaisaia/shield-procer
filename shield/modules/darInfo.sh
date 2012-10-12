#!/bin/bash
if [ -z $CONFSEGURIDAD ]

	then
	echo "No estan cargados los comandos de seguridad"
else
	echo ${CONFSEGURIDAD}
fi
