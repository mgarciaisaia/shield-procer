C_SRCS = ./commons/bitarray.c \
	 ./commons/config.c \
	 ./commons/error.c \
	 ./commons/log.c \
	 ./commons/string.c \
	 ./commons/temporal.c \
	 ./commons/collections/dictionary.c \
	 ./commons/collections/list.c \
	 ./commons/collections/queue.c \
	 ./commons/collections/sync_queue.c \
	 ./commons/collections/stack.c \
	 ./commons/network.c


OBJS =  ./build/commons/bitarray.o \
	./build/commons/config.o \
	./build/commons/error.o \
	./build/commons/log.o \
	./build/commons/string.o \
	./build/commons/temporal.o \
	./build/commons/collections/dictionary.o \
	./build/commons/collections/list.o \
	./build/commons/collections/queue.o \
	./build/commons/collections/sync_queue.o \
	./build/commons/collections/stack.o \
	./build/commons/network.o

all: build/libcommons.so

create-dirs:
	mkdir -p build/commons/collections

build/libcommons.so: create-dirs $(OBJS)
	gcc -shared -o "build/libcommons.so" $(OBJS) -pthread

build/commons/%.o: ../commons/%.c
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

build/commons/collections/%.o: ../commons/collections/%.c
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

clean:
	rm -rf build

.PHONY: all create-dirs clean
