# Default target
help :
	echo "What do you want from me?"

script = shield.sh
rutaScript = $(PWD)/$(script)

esRoot :
	./utils/chequearRoot.sh

instalar : esRoot
	chmod +x $(script)
	ln -s $(ruta) /usr/bin/$(script)

desinstalar : esRoot
	@echo "TODO"

configurar : esRoot
ifdef usuario
	@chsh -s /usr/bin/$(script) $(usuario)
else
	@echo "Falta el usuario"
endif

resetear : esRoot
	echo "TODO"
