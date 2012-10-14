#!/bin/bash
while true
do
	while read modulo
        do
                . $modulo procesar
        done <<< "$1"
	sleep 5
done
