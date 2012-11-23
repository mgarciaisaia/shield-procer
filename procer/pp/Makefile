C_SRCS = ./pp.c \
	 ./parser.c

OBJS = ./build/pp.o \
       ./build/parser.o

all: build/pp

dependents:
	-cd ../commons/ && make all

build/pp: dependents create-dirs $(OBJS)
	gcc -L"../commons/build" -o "build/pp" $(OBJS) -lcommons

create-dirs:
	mkdir -p build

clean-commons:
	-cd ../commons/ && make clean

clean: clean-commons
	rm -rf build

%.o: ../%.c
	gcc -I"../" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# PHONY targets son los que no producen un archivo llamado como el target
# Se los marca para evitar confusion si se crea un archivo llamado asi
.PHONY: all dependents create-dirs clean clean-commons
