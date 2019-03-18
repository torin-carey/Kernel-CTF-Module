obj-m := myflag.o

myflag-objs := flag.o sha256.o

.PHONMY: all

all: modules clean getflag

modules:
	make -C module/ $@

clean:
	make -C module/ $@
	make -C supplied/ $@

getflag:
	make -C supplied/ $@
