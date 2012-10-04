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
	addgroup shield
	echo '%shield	ALL = NOPASSWD: /sbin/halt' > /etc/sudoers.d/42-shield
	chmod 440 /etc/sudoers.d/42-shield

desinstalar : esRoot
	rm /etc/sudoers.d/42-shield
	delgroup shield
	rm /usr/bin/$(script)
	@echo "TODO"d

configurar : esRoot
ifdef usuario
	# TODO : crear la ~/.shield
	# TODO : copiar las config default
	# TODO : crear el archivo de log con permisos
	# TODO : backupear la shell previa para poder restorearla
	@chsh -s /usr/bin/$(script) $(usuario)
	adduser $(usuario) shield
else
	@echo "Falta el usuario"
endif

resetear : esRoot
ifdef usuario
	# TODO : falta restorearle la shell previa
	deluser $(usuario) shield
	# TODO: falta borrar la carpeta .shield
else
	@echo "Falta el usuario"
endif
