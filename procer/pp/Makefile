C_SRCS = ./parser.c \
	 ./procer.c

OBJS = ./build/parser.o \
       ./build/procer.o

all: build/parser

dependents:
	-cd ../commons/ && make all

build/parser: dependents create-dirs $(OBJS)
	gcc -L"../commons" -L"../commons/build" -o "build/parser" $(OBJS) -lcommons -pthread

create-dirs:
	mkdir -p build

clean-all: clean
	-cd ../commons/ && make clean

clean:
	rm -rf build

%.o: ../%.c
	gcc -I"../" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# PHONY targets son los que no producen un archivo llamado como el target
# Se los marca para evitar confusion si se crea un archivo llamado asi
.PHONY: all dependents create-dirs clean
