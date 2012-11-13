C_SRCS = ./commons/bitarray.c \
	 ./commons/config.c \
	 ./commons/error.c \
	 ./commons/log.c \
	 ./commons/string.c \
	 ./commons/temporal.c \
	 ./commons/collections/dictionary.c \
	 ./commons/collections/list.c \
	 ./commons/collections/queue.c


OBJS =  ./build/commons/bitarray.o \
	./build/commons/config.o \
	./build/commons/error.o \
	./build/commons/log.o \
	./build/commons/string.o \
	./build/commons/temporal.o \
	./build/commons/collections/dictionary.o \
	./build/commons/collections/list.o \
	./build/commons/collections/queue.o

all: build/libcommons-library.so

create-dirs:
	mkdir -p build/commons/collections

build/libcommons-library.so: create-dirs $(OBJS)
	gcc -shared -o "build/libcommons-library.so" $(OBJS)

build/commons/%.o: ../commons/%.c
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

build/commons/collections/%.o: ../commons/collections/%.c
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

clean:
	rm -rf build

.PHONY: all create-dirs clean
