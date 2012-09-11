#!/bin/bash
echo "Hola, papu"
who
while true;
do
	read comando
	echo "Ingresaste $comando"
	eval $comando
done
