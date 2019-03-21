obj-m := ctfmod.o

ctfmod-objs := flag.o sha256.o

.PHONY: all

all: modules

modules clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) $@
