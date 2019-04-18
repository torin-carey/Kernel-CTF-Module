obj-m := ctfmod.o

ctfmod-y := ctfmod_main.o sha256.o

ccflags-y := $(CFLAGS)

.PHONY: module modules clean purge

module: ctfmod-stripped.ko

ctfmod-stripped.ko: ctfmod.ko
	strip --strip-unneeded -o $@ $<
	
ctfmod.ko: modules

modules clean purge: 
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) CFLAGS="$(CFLAGS)" $@
