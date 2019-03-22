obj-m := ctfmod.o

ctfmod-objs := flag.o sha256.o

.PHONY: module modules clean purge

module: flag.h modules flag2

flag.h:
	./gen_flag.sh > $@

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	rm -f flag2gen
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

purge: clean
	rm -f flag2 flag.h

flag2gen: flag2gen.c sha256.c
	gcc -o $@ $^

flag2: | flag2gen
	./$| > $@
