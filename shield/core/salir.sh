#!/bin/bash

echo "Seguro desea salir?"
#logout
exit

#Desde Terminal, tienes que hacerlo desde una cuenta de administrador y escribir
#ps -U usuario
#con U mayúscula, lo que te da la lista de procesos activos de usuario.

#Después
#sudo kill -9
#seguido de un espacio y el PID del proceso. De nuevo, al matar loginwindow.app se cierra la sesión.

#Para cierres "suaves" (equivalente a Manzanita+Q) de cada proceso, cambia "kill -9" por "kill -3"
