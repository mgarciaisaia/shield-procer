#!/bin/bash
verbose=1
if [[ ( "--quiet" == $1 ) || ( "-s" == $1 ) ]] # ) -o ( $1 = "-s" ) ]]
then
	verbose=0
fi

algunoConfigurado=0
mensaje="Ningún usuario tiene SHIELD configurado"

while read linea
do
	userId=$(echo $linea | cut -d: -f3 -)
	username=$(echo $linea | cut -d: -f1 -)
	if [ $userId -ge 1000 ]
	then
		userHome=$(echo $linea | cut -d: -f6 -)
		if [ -d $userHome/.shield ]
		then
			mensaje="El usuario $username esta configurado"
			algunoConfigurado=1
		fi
	fi
done < /etc/passwd

if [ $verbose -ne 0 ]; then
	echo $mensaje
fi
exit $algunoConfigurado
