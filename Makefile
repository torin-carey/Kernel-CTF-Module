obj-m := myflag.o

myflag-objs := flag.o sha256.o

.PHONMY: all

all: modules getflag

modules:
	make -C module/ $@
	ln -s module/flagmod.ko .

clean:
	make -C module/ $@
	make -C supplied/ $@
	rm -f flagmod.ko

getflag:
	make -C supplied/ $@
