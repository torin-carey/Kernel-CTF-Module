obj-m := myflag.o

myflag-objs := flag.o sha256.o

.PHONMY: all

all: modules getflag

modules:
	make -C module/ $@
	ln -fs module/ctfmod.ko .

clean:
	make -C module/ $@
	make -C solutions/ $@
	rm -f ctfmod.ko

getflag:
	make -C solutions/ $@
