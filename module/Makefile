obj-m := ctfmod.o

ctfmod-objs := flag.o sha256.o

.PHONY: all

all: flag.h modules

flag.h:
	./gen_flag.sh > $@

modules clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) $@
