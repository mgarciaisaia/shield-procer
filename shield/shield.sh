#!/bin/bash
echo "Hola, papu"
who
while true;
do
	read -e comando
	echo "Ingresaste $comando"
	eval $comando
done
