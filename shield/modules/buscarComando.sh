BUSQUEDA=`echo ${CONFSEGURIDAD} | grep -o -w "${1}"`
if [ -z $BUSQUEDA ];
then
exit 0
else
echo ERROR!!
fi
