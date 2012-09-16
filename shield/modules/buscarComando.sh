CONFIGURACION=/home/utnso/tpos/2012-2c-no-quiero-matarte-pero-si-me-obligas/shield/configuraciones/seguridad.conf
BUSQUEDA=$(grep -c $1 $CONFIGURACION )
if [ $BUSQUEDA != 0 ]
then
echo ERROR!!
fi
