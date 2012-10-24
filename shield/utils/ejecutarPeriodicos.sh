#!/bin/bash
while true
do
	for modulo in $@
        do
                . $modulo procesar
        done
	sleep 5
done
