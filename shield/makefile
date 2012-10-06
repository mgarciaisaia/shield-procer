# Default target
help :
	@echo "What do you want from me?"

script = shield.sh
rutaScript = $(shell pwd)/$(script)
enlaceScript = /usr/bin/$(script)

esRoot :
	@./utils/chequearRoot.sh --quiet

usuarioIngresado:
ifndef usuario
	@echo "Falta el usuario"
	@echo '  **Uso: sudo make usuario=$$USUARIO resetear'
	@exit 1
endif

usuarioConfigurado: usuarioIngresado
	@test -e ~$(usuario)/.shield

instalar : esRoot
	@chmod +x $(script)
	@ln -s $(rutaScript) $(enlaceScript) 
	@addgroup shield
	@echo '%shield	ALL = NOPASSWD: /sbin/halt' > /etc/sudoers.d/42-shield
	@chmod 440 /etc/sudoers.d/42-shield

desinstalar : esRoot
	@rm /etc/sudoers.d/42-shield
	@delgroup shield
	@rm $(enlaceScript) 

configurar : esRoot usuarioIngresado
	@mkdir ~$(usuario)/.shield # Si tenia configurado, rompe
	@# TODO : copiar las config default
	@# Archivo de log de errores de la shell (**NO** es auditoria)
	@touch ~$(usuario)/.shield/shell.log
	@# TODO : SHIELD tiene que poder escribir en shell.log y en el log de auditoria
	@#chown $(usuario) ~$(usuario)/.shield/shell.log
	@# Backupeo la shell actual (7ma entrada del /etc/passwd separado por : )
	@getent passwd $(usuario) | cut -d: -f7 > ~$(usuario)/.shield/previous_shell
	@chsh -s $(enlaceScript) $(usuario)
	@adduser $(usuario) shield

resetear : esRoot usuarioIngresado usuarioConfigurado
	@chsh -s `cat ~$(usuario)/.shield/previous_shell` $(usuario)
	@deluser $(usuario) shield
	@rm -rf ~$(usuario)/.shield
