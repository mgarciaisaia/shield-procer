#!/bin/bash
idUsuario=$(id -u)
if (( $idUsuario )); then
	echo "No sos root :("
else
	echo "Root para todos!! :D";
fi
