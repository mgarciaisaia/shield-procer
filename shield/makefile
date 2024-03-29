# Default target
help :
	@echo "♫ What do you want from me? ♫"

script = shield.sh
rutaScript = $(destino)/$(script)
enlaceScript = /usr/bin/$(script)

esRoot :
	@./utils/chequearRoot.sh --quiet

hola :
	@echo $(destino)
	@echo $(rutaScript)

usuarioIngresado:
ifndef usuario
	@echo "Falta el usuario"
	@echo '  **Uso: sudo make usuario=$$USUARIO resetear'
	@exit 1
endif

usuarioConfigurado: usuarioIngresado
	@test -d ~$(usuario)/.shield

usuarioNoConfigurado: usuarioIngresado
	@test ! -d ~$(usuario)/.shield

noEstaInstalado:
	@test ! -e $(enlaceScript)

estaInstalado:
	@test -e $(enlaceScript)

nadieConfigurado:
	@./utils/nadieConfigurado.sh --quiet

ifndef destino
destino = /etc/shield
else
destino = $(destino:/=)
endif

instalar : esRoot noEstaInstalado
	@cp -R . $(destino)
	@cd $(destino)
	@chmod +x $(rutaScript)
	@ln -s $(rutaScript) $(enlaceScript) 
	@addgroup shield
	@echo '%shield	ALL = NOPASSWD: /sbin/halt, /bin/ls -l /proc/*' > /etc/sudoers.d/42-shield
	@chmod 440 /etc/sudoers.d/42-shield
	@ssh-keygen -q -t rsa -C "SHIELD's key" -f $(destino)/clave_auditoria -N ''
	@chown root:shield $(destino)/clave_auditoria
	@chmod g+r $(destino)/clave_auditoria
	@echo $(enlaceScript) >> /etc/shells

desinstalar : esRoot estaInstalado nadieConfigurado
	@rm /etc/sudoers.d/42-shield
	@delgroup shield
	@./utils/removerShieldShells.sh
	@rm -rf $(shell dirname $$(readlink $(enlaceScript)))
	@rm $(enlaceScript) 

configurar : esRoot usuarioNoConfigurado
	@./utils/copiarClaveSsh.sh $(enlaceScript) $(usuario)
	@mkdir ~$(usuario)/.shield # Si tenia configurado, rompe
	@cp -r configuraciones/ ~$(usuario)/.shield/conf
	@# Archivo de log de errores de la shell (**NO** es auditoria)
	@touch ~$(usuario)/.shield/shell.log ~$(usuario)/.shield/auditoria.log
	@chown $(usuario) ~$(usuario)/.shield/shell.log ~$(usuario)/.shield/auditoria.log
	@# Backupeo la shell actual (7ma entrada del /etc/passwd separado por : )
	@getent passwd $(usuario) | cut -d: -f7 > ~$(usuario)/.shield/previous_shell
	@chsh -s $(enlaceScript) $(usuario)
	@adduser $(usuario) shield

resetear : esRoot usuarioIngresado usuarioConfigurado
	@chsh -s `cat ~$(usuario)/.shield/previous_shell` $(usuario)
	@deluser $(usuario) shield
	@rm -rf ~$(usuario)/.shield
