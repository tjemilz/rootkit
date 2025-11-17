CC = gcc
CFLAGS_SO = -fPIC -shared -ldl
CFLAGS_MAIN = 

all: hook.so main

hook.so: hook.c
	$(CC) $(CFLAGS_SO) hook.c -o hook.so

main: main.c
	$(CC) main.c -o main

test: all
	LD_PRELOAD=./hook.so ./main

clean:
	rm -f hook.so main

.PHONY: all clean test
