obj-m += level2.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod level2.ko

unload:
	sudo rmmod level2

test: all load
	dmesg | tail -5

.PHONY: all clean load unload test
