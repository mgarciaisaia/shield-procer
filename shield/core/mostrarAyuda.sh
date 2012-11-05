#!/bin/bash

#Pregunto si tiene parametros y si no muestro ayuda de todos los builtins

AYUDA="Brinda ayuda sobre el buit-in ingresado, de no ingresar ninguno, brinda ayuda de todos los  built-ins"
INFOM="Muestra información sobre los módulos que contengan la cadena ingresada, de no ingresar ninguna cadena,  muestra información sobre todos los módulos"
LISTAR="Lista el path absolutos de los modulos que tiene activos"
ACTUAL="Registra e inicializa los módulos del usuario"
MOSTRAR="Muestra el contenido de esa variable interna del shell"
SALIR="Termina la sesión actual"
APAGAR="Apaga el sistema"

if [ -z $1 ] 
then
	#mostrar ayuda de todos los buitins
	echo "ayuda <buit-in> :	" $AYUDA
	echo "info_modulos <cadena> :	" $INFOM
	echo "listar_modulos :	" $LISTAR
	echo "actualizar_modulos :	" $ACTUAL
	echo "mostrar variable :	" $MOSTRAR
	echo "salir :			" $SALIR
	echo "apagar :		" $APAGAR
else

case "$1" in
ayuda)	echo $AYUDA
	;;
info_modulos) echo $INFOM
	;;
listar_modulos) echo $LISTAR
	;;
actualizar_modulos)  echo $ACTUAL
	;;
mostrar) echo $MOSTRAR
	;;
salir) echo $SALIR
	;;
apagar) echo $APAGAR
	;;
esac

fi
