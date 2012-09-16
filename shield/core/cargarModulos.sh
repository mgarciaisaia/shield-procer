#!/bin/bash
while read modulo
do
	echo $modulo
done <<"EOF"
echo
ping -c 2
EOF
