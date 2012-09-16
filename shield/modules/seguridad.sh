case $1 in
	informacion) echo "one" ;;
	iniciar) echo "two" ;;
	detener) echo "three" ;;
	procesar) echo $2;;
	*) echo "No se que es eso.." ;;
esac
